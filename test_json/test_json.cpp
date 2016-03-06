#include "json11.hpp"
#include "reflection.h"

#define THROW_EXCEPTION(name, msg) { name ex; ex.What() = msg; ex.Where() = std::string(__FILE__ " (" + std::to_string(__LINE__) + ")"); throw ex; }

using namespace std;

class PhoneNumber{
    BEGIN_TYPE(PhoneNumber)
        FIELDS(FIELD(&PhoneNumber::areaCode), FIELD(&PhoneNumber::number))
        CTORS(DEFAULT_CTOR(PhoneNumber), CTOR(PhoneNumber, const std::string&, const std::string&))
        METHODS(METHOD(&PhoneNumber::ToString))
    END_TYPE
public:
    std::string areaCode;
    std::string number;

    PhoneNumber() {}
    PhoneNumber(const std::string& areaCode, const std::string& number) : areaCode(areaCode), number(number) {}
    
    std::string ToString() const{
        return areaCode + " " + number;
    }
};

REFLECT_ENUM(Sex, Secret, Male, Female)

class Person{
    BEGIN_TYPE(Person)
        FIELDS(FIELD(&Person::name), FIELD(&Person::height), FIELD(&Person::sex), FIELD(&Person::phoneNumber), FIELD(&Person::totalNumber))
        CTORS(DEFAULT_CTOR(Person), CTOR(Person, const std::string&, float, Sex))
        METHODS(METHOD(&Person::GetName), METHOD(&Person::SetName), METHOD(&Person::Name), METHOD(&Person::GetHeight), METHOD(&Person::GetSex), METHOD(&Person::GetPhoneNumber), METHOD(&Person::SetPhoneNumber), METHOD(&Person::GetTotalNumber), METHOD(&Person::IsMale), METHOD(&Person::IsFemale))
    END_TYPE
protected:
    std::string name;
    float height;
    Sex sex;
    PhoneNumber phoneNumber;

    static int totalNumber;

public:
    Person(const std::string& name, float height, Sex sex)
        : name(name), height(height), sex(sex) {
        totalNumber++;
    }

    Person() : Person("Unnamed", 0, Sex::Secret) { }

    const std::string& GetName() const{
        return name;
    }

    void SetName(const std::string& name){
        this->name = name;
    }

    std::string& Name() {
        return name;
    }

    float GetHeight() const{
        return height;
    }

    Sex GetSex() const{
        return sex;
    }

    bool IsMale() const{
        return sex == Sex::Male;
    }

    bool IsFemale() const{
        return sex == Sex::Female;
    }

    const PhoneNumber& GetPhoneNumber(){
        return phoneNumber;
    }

    void SetPhoneNumber(const PhoneNumber& phoneNumber){
        this->phoneNumber = phoneNumber;
    }

    static int GetTotalNumber(){
        return totalNumber;
    }

};

int Person::totalNumber = 0;

using namespace json11;

Any FromJson(const Json& json, const Type* type){
#define XX(T)    if (type == typeof(T)) { return T(json.number_value()); }
    XX(int8_t);
    XX(int16_t);
    XX(int32_t);
    XX(int64_t);
    XX(uint8_t);
    XX(uint16_t);
    XX(uint32_t);
    XX(uint64_t);
    XX(float);
    XX(double);
    if (type == typeof(bool)){ return json.bool_value(); }
    if (type == typeof(std::string)) { return json.string_value(); }
    if (type->IsEnum()){ return Enum::GetValue(type, json.string_value()); }
    
    Any obj = type->GetConstructor()->Invoke();
    for (auto& i : json.object_items()){
        auto field = type->GetField(i.first);
        if (!field) THROW_EXCEPTION(FieldNotFound, "field '" + field->GetName() + "' not found in class '" + type->GetName() + "'");
        auto value = FromJson(i.second, field->GetType().GetType());
        for (int i = value.GetType().PointerCount() - field->GetType().PointerCount(); i > 0; i--)
            value = *value;
        field->Set(obj, value);
    }
    return obj;
}

template<class T>
std::shared_ptr<T> FromJson(const Json& json){
    return std::shared_ptr<T>((T*)FromJson(json, typeof(T)));
}

class JsonError : public Exception{};  

template<class T>
std::shared_ptr<T> FromJson(const std::string& jsonText){
    std::string err;
    auto json = Json::parse(jsonText, err);
    if (err.empty())
        return FromJson<T>(json);

    THROW_EXCEPTION(JsonError, "failed to parse json: " + jsonText);
}

template<class T>
std::shared_ptr<T> FromJson(const char* jsonText){
    return FromJson<T>(std::string(jsonText));
}

std::string ToJson(void* obj, const Type* type){
    std::stringstream ss;

    if (obj == nullptr){
        ss << "null";
    }
    else{
        ss << "{ ";
        int i = 0;
        for (auto f : type->GetMemberFields()){
            if (i++ > 0) ss << ", ";
            ss << "\"" << f->GetName() << "\" : ";

            auto ft = f->GetType();
            if (ft.IsNumber() || ft.IsBool())
                ss << f->Get(obj).ToString();
            else if (ft.IsString())
                ss << "\"" << f->Get(obj).ToString() << "\"";
            else if (ft.IsEnum())
                ss << "\"" << Enum::GetName(ft.GetType(), (int64_t)f->Get(obj)) << "\"";
            else
                ss << ToJson((void*)f->Get(obj), ft.GetType());
        }
        ss << " }";
    }
    
    return ss.str();
}

template<class T>
std::string ToJson(T& obj){
    auto type = qualified_typeof(T).GetType();
    return ToJson(&obj, qualified_typeof(T).GetType());
}

template<class T>
std::string ToJson(T* obj){
    return ToJson(obj, qualified_typeof(T).GetType());
}

template<class T>
std::string ToJson(std::shared_ptr<T> obj){
    return ToJson<T>(*obj);
}

int main(){
    try{
        auto p = FromJson<Person>(R"({"name":"John", "height":1.7, "sex":"Female", "phoneNumber" : {"areaCode":"+86", "number":"13888888888"}})");
        auto newPhone = Type::GetType("PhoneNumber")->GetConstructor({qualified_typeof(const std::string&), qualified_typeof(const std::string&)})->Invoke(std::string("+86"), std::string("13000000000"));
        p->GetType()->GetMethod("SetPhoneNumber")->Invoke(p.get(), newPhone);
        std::cout << ToJson(p) << std::endl;
        PhoneNumber phone = p->GetType()->GetMethod("GetPhoneNumber")->Invoke(p.get());
        std::cout << phone.ToString() << endl;
        Sex sex = p->GetType()->GetField("sex")->Get(p.get());
        std::cout << Enum::GetName(sex) << std::endl;
        
        std::cout << typeof(Person)->GetDescription() << std::endl;
        std::cout << typeof(PhoneNumber)->GetDescription() << std::endl;

        auto type1 = p->GetType();
        auto type2 = typeof(Person);
        auto type3 = Person::StaticType();
        auto type4 = Type::GetType("Person");

        short i = 1;
        Any any1(i);
        Any any2(any1);
        cout << (int)any1 << ", " << (int)any2 << endl;
        any2 = any1;
        cout << (int)any1 << ", " << (int)any2 << endl;
        any2 = Any(2);
        cout << (int)any2 << endl;
        any2 = 1;
        cout << (int)any2 << endl;


#define TEST(x)     cout << Any(x).GetType().ToString() << " : " << Any(x).Cast<decltype(x)>() << endl;
        TEST(i);
        TEST(3.14);
        cout << Any(1.5).Cast<int>() << endl;
        cout << Any("foo").GetType().ToString() << " : " << Any("foo").Cast<const char*>() << endl;
        TEST(std::string("bar"));
        TEST(make_shared<string>("asdfgh12345"));
        string a = "foo";
        string& ref = a;
        int b = 123;
        TEST(b);
        auto any = Any(a);
        any.Cast<string&>().append("_modified");
        Any aa = Any(a);
        cout << any.GetType().ToString() << " : " << any.Cast<string>() << ", " << ref << endl;
        aa.Cast<string&>().append("123");
        cout << aa.Cast<const string&>() << endl;
        //TEST(ref);
    }
    catch (const Exception& ex){
        std::cout << "error: " << ex.What() << "  at: " << ex.Where() << std::endl;
    }

    system("pause");

    return 0;
}