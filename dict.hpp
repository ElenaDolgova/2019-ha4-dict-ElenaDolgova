#pragma once

#include <map>
#include <cstdio>
#include <utility>
#include <stdexcept>
#include <string>
#include <any>
#include <iostream>
#include <unordered_map>

namespace utils {


    class no_key_exception : public std::runtime_error {
    public:
        no_key_exception()
                : runtime_error("no_key_exception") {}
    };

    class invalid_type_exception : public std::runtime_error {
    public:
        invalid_type_exception()
                : runtime_error("invalid_type_exception") {}

    };

    struct objectI {
        virtual std::unique_ptr<objectI> clone() const = 0;

        virtual const std::type_info &type() const = 0;

        virtual ~objectI() {};
    };

    template<typename T>
    struct object : public objectI {
        T value;

        object(T &&v) : value(std::move(v)) {}

        object(const T &v) : value(v) {}

        virtual std::unique_ptr<objectI> clone() const override {
            return std::unique_ptr<objectI>(new object<T>(value));
        }

        const std::type_info &type() const override {
            return typeid(T);
        }
    };

    class anyObject {

    public:
        anyObject() : ptr(nullptr) {}

        anyObject(anyObject &&x) : ptr(std::move(x.ptr)) {}

        template<class T>
        anyObject(const T &x)
                : ptr(new object<typename std::decay<const T>::type>(x)) {}

        template<class T>
        friend T any_cast(anyObject &);

        template<class Type>
        friend Type any_cast(const anyObject &);

        template<class Type>
        friend Type *any_cast(anyObject *);

        template<class Type>
        friend const Type *any_cast(const anyObject *);

        anyObject &operator=(anyObject &&rhs) {
            ptr = std::move(rhs.ptr);
            return (*this);
        }

        anyObject(const anyObject &x) {
            if (x.ptr)
                ptr = x.ptr->clone();
        }

        anyObject &operator=(const anyObject &rhs) {
            ptr = std::move(anyObject(rhs).ptr);
            return (*this);
        }

        template<class T>
        anyObject &operator=(T &&x) {
            ptr.reset(new object<typename std::decay<T>::type>(typename std::decay<T>::type(x)));
            return (*this);
        }

        template<class T>
        anyObject &operator=(const T &x) {
            ptr.reset(new object<typename std::decay<T>::type>(typename std::decay<T>::type(x)));
            return (*this);
        }

        std::unique_ptr<objectI> ptr;
    };

    template<class Type>
    Type any_cast(anyObject &val) {
        std::type_info const &info = typeid(Type);
        std::unique_ptr<objectI>::pointer pI = val.ptr.get();
        std::type_info const &typeInfo = pI->type();
        if (typeInfo != info)
            throw invalid_type_exception();
        return static_cast<object<Type> *>(val.ptr.get())->value;
    }

    template<class Type>
    Type any_cast(const anyObject &val) {
        return any_cast<Type>(anyObject(val));
    }

    template<typename Type>
    Type *any_cast(anyObject *ptr) {
//        return static_cast<T*>(dict.hash_map.at(key));
//        return dynamic_cast<Type*>(ptr->ptr.get());

        std::unique_ptr<objectI>::pointer pI = ptr->ptr.get();
        auto *pObject = static_cast<object<Type> *>(pI);
        return &pObject->value;
    }

    template<typename Type>
    const Type *any_cast(const anyObject *ptr) {
        return dynamic_cast<const Type *>(ptr->ptr.get());
    }

    struct dict_t {
        std::unordered_map<std::string, dict_t> hash_map;
        anyObject value;

        dict_t() = default;

        //                dict_t dict = l.second;
//                if (dict.hash_map.size() == 1 && dict.hash_map.count("") == 1) {
//                    hash_map[l.first] = dict.hash_map[""];
//                } else {
//                    hash_map[l.first] = dict;
//                }
//                anyObject ob = l.second;

        dict_t(std::initializer_list<std::pair<const std::string, dict_t>> list) {
            for (const auto &l:list) {
//                if (l.second.hash_map.size() > 1) {
//                    std::cout << " in list " << l.first << std::endl;
//                } else {
//                    dict_t dict = l.second;
//                    anyObject &object = dict.hash_map[""];
//                    std::unique_ptr<objectI>::pointer pI = object.ptr.get();
//                    std::type_info const &typeInfo = pI->type();
//                    if(typeInfo!= typeInfo){}
//                    std::cout << "hghg " << l.first << std::endl;
//                }
                hash_map[l.first] = l.second;
            }
        }

        template<typename T>
        dict_t(T val) {
            value = val;
        }

        bool operator==(const dict_t &rhs) const {
            if (hash_map.size() != rhs.hash_map.size()) {
                return false;
            } else {

                for (const auto &elem: hash_map) {
                    if (rhs.hash_map.count(elem.first) == 0) {
                        return false;
                    }
                }
            }

            return true;
        }
    };

    template<typename T>
    static bool put(dict_t &dict, const std::string &key, T value) {
        if (dict.hash_map.count(key) == 0) {
            dict.hash_map[key] = value;
            return true;
        }
        return false;
    }

    template<typename T>
    static T get(const dict_t &dict, const std::string &key) { // вот тут обрати внимание, нужна ссылка
        anyObject v = 1;
        try {
            anyObject d = dict.hash_map.at(key);
            v = dict.hash_map.at(key);
        } catch (std::out_of_range) {
            throw no_key_exception();// чего??? фигня какая-то
        }
        try {
            return any_cast<T>(v);
        } catch (std::bad_any_cast) {
            throw invalid_type_exception();
        }
    }

    template<typename T>
    static T *get_ptr(const dict_t &dict, const std::string &key) {
        anyObject a = nullptr;

        try {
            anyObject ptr = dict.hash_map.at(key);
            return any_cast<T>(&ptr);
            a = dict.hash_map.at(key);
        } catch (std::out_of_range) {
            return nullptr;
        }
        return any_cast<T>(&a);
    }

    template<typename T>
    static bool remove(const dict_t &dict, const std::string &key) {
        return true;
    }

    template<typename T>
    static bool contains(const dict_t &dict, const std::string &key) {

    }

    static bool empty(dict_t dict) {
        return true;
    }

//    static void clear(const dict_t &dict, const std::string &key) {
//
//    }

//    static bool is_dict(dict_t dict) {
//        return true;
//    }
}
