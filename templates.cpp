//
// Created by Dima on 06.03.2021.
//

#include "templates.h"



void TemplatesTest() {
    //IteratorTest();
    SliceTest();
    ZipTest();
    TypeTraitsTest();
    {
        {
            std::vector<int> v{1, 2, 3, 4, 5};
            double average_ref = 3;
            double average = get_average(v);
            ASSERT_EQUAL(average, average_ref);
        }
        {
            std::vector<double> v{1.5, 2.5, 3.5, 4.5, 5.5};
            double average_ref = 3.5;
            double average = get_average(v);
            ASSERT_EQUAL(average, average_ref);
        }
    }
    {
        {
            std::vector<std::vector<int>> v = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}};
            std::vector<std::vector<int>> res_ref = {{1, 3, 5, 7, 9}, {2, 4, 6, 8, 10}};
            auto res = vector_transpose<int>(v);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            std::vector<std::vector<double>> v = {{1.5, 2}, {3.5, 4}, {5.5, 6}, {7.5, 8}, {9.5, 10}};
            std::vector<std::vector<double>> res_ref = {{1.5, 3.5, 5.5, 7.5, 9.5}, {2, 4, 6, 8, 10}};
            auto res = vector_transpose<double>(v);
            ASSERT_EQUAL(res, res_ref);
        }
        {
            std::vector<std::vector<TestValue>> v = {{TestValue(1), TestValue(2)}, {TestValue(3), TestValue(4)}};
            std::vector<std::vector<int>> res_ref = {{1, 3}, {2, 4}};
            auto res = vector_transpose<TestValue>(v);
            for(const auto&[_res, _res_ref] : zip(res, res_ref)) {
                for(const auto&[__res, __res_ref] : zip(_res, _res_ref)) {
                    ASSERT_EQUAL(__res.get(), __res_ref);
                }
            }
        }
    }
}

//void IteratorTest(){
//    std::vector<TestValue*> v {new TestValue(1), new TestValue(2), new TestValue(3)};
//    {
//        std::vector<TestValue*> res;
//        for(const auto& i : IntValueIterator(v)) {
//            res.emplace_back(i);
//        }
//        ASSERT_EQUAL(res, v);
//        for(const auto& i : IntValueIterator(v)) {
//            delete i;
//        }
//        v.clear();
//    }
//}

void SliceTest() {
    //void slice(std::vector<T> const &v, std::pair<int, int> const p, std::vector<T> &res)
    std::vector<int> v {1,2,3,4,5,6,7,8,9,10};
    {
        std::vector<int> s {1,2,3,4,5};
        std::pair<int,int> p {0,5};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {6,7,8,9,10};
        std::pair<int,int> p {5,10};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {4,5,6,7};
        std::pair<int,int> p {3,7};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {};
        std::pair<int,int> p {0,0};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {1};
        std::pair<int,int> p {0,1};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {10};
        std::pair<int,int> p {9,10};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {};
        std::pair<int,int> p {-1,0};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {6,7,8,9,10};
        std::pair<int,int> p {5,12};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {};
        std::pair<int,int> p {12,15};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {};
        std::pair<int,int> p {10,11};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {};
        std::pair<int,int> p {5,5};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
    {
        std::vector<int> s {};
        std::pair<int,int> p {7,5};
        std::vector<int> res;
        slice(v, p, res);
        ASSERT_EQUAL(res, s);
    }
}

void ZipTest() {
    {
        std::vector<std::tuple<int, int>> t = {{1,6},{2,7},{3,8},{4,9},{5,10}};
        auto res = zip<int, int>({1,2,3,4,5}, {6,7,8,9,10});
        ASSERT_EQUAL(res, t);
    }
    {
        std::vector<std::tuple<int, std::string>> t = {{1,"a"},{2,"b"},{3,"c"},{4,"d"},{5,"e"}};
        auto res = zip<int, std::string>({1,2,3,4,5}, {"a","b","c","d","e"});
        ASSERT_EQUAL(res, t);
    }
    {
        std::vector<std::tuple<int, std::string>> t = {{1,"a"},{2,"b"},{3,"c"},{4,"d"},{5,"e"}};
        auto res = zip<int, std::string>({1,2,3,4,5}, {"a","b","c","d","e"});
        ASSERT_EQUAL(res, t);
    }
    {
        std::vector<std::tuple<int, std::string, double>> t = {{1,"a",1.5},{2,"b",2.5},{3,"c",3.5},{4,"d",4.5},{5,"e",5.5}};
        auto res = zip<int, std::string, double>({1,2,3,4,5}, {"a","b","c","d","e"}, {1.5,2.5,3.5,4.5,5.5});
        ASSERT_EQUAL(res, t);
    }

    {
        std::vector<int> v1 {1,2,3,4,5};
        std::vector<std::string> v2 {"a", "b", "c", "d", "e"};
        std::vector<double> v3 {1.5,2.5,3.5,4.5,5.5};
        auto res = zip(v1, v2, v3);

        std::vector<int> vres1;
        std::vector<std::string> vres2;
        std::vector<double> vres3;

        for(const auto&[i, s, d] : res) {
            vres1.emplace_back(i);
            vres2.emplace_back(s);
            vres3.emplace_back(d);
        }
        ASSERT_EQUAL(vres1, v1);
        ASSERT_EQUAL(vres2, v2);
        ASSERT_EQUAL(vres3, v3);
    }
    {
        std::vector<int> v1 {1,2,3,4,5};
        std::vector<std::string> v2 {"a", "b", "c", "d", "e"};
        std::vector<double> v3 {1.5,2.5,3.5,4.5,5.5};

        std::vector<int> vres1;
        std::vector<std::string> vres2;
        std::vector<double> vres3;

        for(const auto&[i, s, d] : zip(v1, v2, v3)) {
            vres1.emplace_back(i);
            vres2.emplace_back(s);
            vres3.emplace_back(d);
        }

        ASSERT_EQUAL(vres1, v1);
        ASSERT_EQUAL(vres2, v2);
        ASSERT_EQUAL(vres3, v3);
    }
    {
        std::vector<std::tuple<int, std::string, double>> t1 = {{1,"a",1.5},{2,"b",2.5},{3,"c",3.5}};
        std::vector<std::tuple<int, std::string, double>> t2 = {{1,"a",1.5}};
        std::vector<int> v1 {1,2,3};
        std::vector<std::string> v2 {"a", "b", "c", "d", "e"};
        std::vector<double> v3 {1.5,2.5,3.5,4.5,5.5};
        std::vector<double> v4 {1.5};
        auto res1 = zip(v1, v2, v3);
        auto res2 = zip(v1, v2, v4);
        ASSERT_EQUAL(res1, t1);
        ASSERT_EQUAL(res2, t2);
    }
    {
        std::vector<std::tuple<int>> t1 {};
        std::vector<std::tuple<int, std::string>> t2 {};
        std::vector<int> v1 {};
        std::vector<std::string> v2 {};
        auto res1 = zip(v1);
        auto res2 = zip(v1, v2);
        ASSERT_EQUAL(res1, t1);
        ASSERT_EQUAL(res2, t2);
    }
    {
        std::vector<int> v1 {1,2,3,4,5};
        std::vector<std::string> v2 {"a", "b", "c", "d", "e"};
        std::vector<std::vector<int>> v3 {{0,0},{0,1},{1,0},{1,1},{1,1,1,1,1,1}};

        std::vector<int> vres1;
        std::vector<std::string> vres2;
        std::vector<std::vector<int>> vres3;

        for(const auto&[i, s, d] : zip(v1, v2, v3)) {
            vres1.emplace_back(i);
            vres2.emplace_back(s);
            vres3.emplace_back(d);
        }

        ASSERT_EQUAL(vres1, v1);
        ASSERT_EQUAL(vres2, v2);
        ASSERT_EQUAL(vres3, v3);
    }
    {

        std::vector<int> v1 {1,2,3};
        std::vector<TestValue> v2 {TestValue(1), TestValue(2), TestValue(3)};

        std::vector<int> vres1;
        std::vector<TestValue> vres2;

        for(const auto&[i, o] : zip(v1, v2)) {
            vres1.emplace_back(i);
            vres2.emplace_back(o);
            ASSERT_EQUAL(i, o.get());
        }

        ASSERT_EQUAL(vres1, v1);
        //ASSERT_EQUAL(vres2, v2); //no friend << for local class
    }
}

void TypeTraitsTest() {
    {
        //type_traits
        class Base {};
        class Derived : public Base {};
        Base b;
        Derived d;

        ASSERT_EQUAL(std::is_integral_v<int>, true);
        ASSERT_EQUAL(std::is_integral_v<std::string>, false);

        ASSERT_EQUAL((std::is_same_v<decltype(b), Base>), true);
        ASSERT_EQUAL((std::is_base_of_v<Base, decltype(d)>), true);
    }
    {
        //custom type traits
        ClassRenderable renderable;
        ClassNotRenderable not_renderable;

        ASSERT_EQUAL(check_renderable(renderable), true);
        ASSERT_EQUAL(check_renderable(not_renderable), false);
    }
}
