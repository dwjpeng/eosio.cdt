#include <eosio/eosio.hpp>

struct my_struct {
   eosio::name n1;
   std::string foo;
   uint64_t bar;
   int32_t baz;

   auto primary_key() const { return eosio::make_key(n1); }
   auto foo_key() const { return eosio::make_key(foo); }
   auto bar_key() const { return eosio::make_key(bar); }
   auto baz_key() const { return eosio::make_key(baz); }
   auto foo_i_key() const { return eosio::make_key(foo, true); }

   bool operator==(const my_struct b) const {
      return n1 == b.n1 &&
             foo == b.foo &&
             bar == b.bar &&
             baz == b.baz;
   }
};

struct my_table : eosio::kv_table<my_struct> {
   eosio::kv_table<my_struct>::index primary_index{eosio::name{"primary"}, &my_struct::primary_key};
   eosio::kv_table<my_struct>::index foo_index{eosio::name{"foo"}, &my_struct::foo_key};
   eosio::kv_table<my_struct>::index bar_index{eosio::name{"bar"}, &my_struct::bar_key};
   eosio::kv_table<my_struct>::index baz_index{eosio::name{"baz"}, &my_struct::baz_key};
   eosio::kv_table<my_struct>::index ifoo_index{eosio::name{"ifoo"}, &my_struct::foo_i_key};

   my_table() {
      init(eosio::name{"kvtest"}, eosio::name{"table"}, &primary_index, &foo_index, &bar_index, &baz_index, &ifoo_index);
   }
};

class [[eosio::contract]] kv_multiple_indices_tests : public eosio::contract {
public:
   using contract::contract;
   my_struct s{
      .n1 = "bob"_n,
      .foo = "a",
      .bar = 5,
      .baz = 0
   };
   my_struct s2{
      .n1 = "alice"_n,
      .foo = "C",
      .bar = 4,
      .baz = -1
   };
   my_struct s3{
      .n1 = "john"_n,
      .foo = "e",
      .bar = 3,
      .baz = -2
   };
   my_struct s4{
      .n1 = "joe"_n,
      .foo = "g",
      .bar = 2,
      .baz = 1
   };
   my_struct s5{
      .n1 = "billy"_n,
      .foo = "I",
      .bar = 1,
      .baz = 2
   };

   [[eosio::action]]
   void setup() {
      my_table t;

      t.upsert(s);
      t.upsert(s2);
      t.upsert(s3);
      t.upsert(s4);
      t.upsert(s5);
   }

   [[eosio::action]]
   void find() {
      my_table t;

      auto itr = t.primary_index.find("bob"_n);
      auto val = itr.value();
      eosio::check(val.n1 == "bob"_n, "Got the wrong n1");

      itr = t.foo_index.find("C");
      val = itr.value();
      eosio::check(val.n1 == "alice"_n, "Got the wrong n1");

      itr = t.bar_index.find((uint64_t)1);
      val = itr.value();
      eosio::check(val.n1 == "billy"_n, "Got the wrong n1");

      itr = t.baz_index.find(0);
      val = itr.value();
      eosio::check(val.n1 == "bob"_n, "Got the wrong n1");

      itr = t.baz_index.find(-1);
      val = itr.value();
      eosio::check(val.n1 == "alice"_n, "Got the wrong n1");

      itr = t.baz_index.find(2);
      val = itr.value();
      eosio::check(val.n1 == "billy"_n, "Got the wrong n1");

      itr = t.baz_index.find(1);
      val = itr.value();
      eosio::check(val.n1 == "joe"_n, "Got the wrong n1");

      itr = t.baz_index.find(-2);
      val = itr.value();
      eosio::check(val.n1 == "john"_n, "Got the wrong n1");
   }

   [[eosio::action]]
   void iteration() {
      my_table t;

      auto begin_itr = t.foo_index.begin();
      auto end_itr = t.foo_index.end();

      // operator++ (case sensitive string)
      // ----------
      auto itr = t.foo_index.begin();
      eosio::check(itr != end_itr, "Should not be the end");
      eosio::check(itr.value().foo == "C", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "I", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "a", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "e", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "g", "Got the wrong value");
      ++itr;
      eosio::check(itr == end_itr, "Should be the end");

      // operator-- (case sensitive string)
      // ----------
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr == begin_itr, "Should be the beginning");

      // operator int32_t
      // ----------------
      itr = t.baz_index.begin();
      eosio::check(itr.value().baz == -2, "Got the wrong value");
      ++itr;
      eosio::check(itr.value().baz == -1, "Got the wrong value");
      ++itr;
      eosio::check(itr.value().baz == 0, "Got the wrong value");
      ++itr;
      eosio::check(itr.value().baz == 1, "Got the wrong value");
      ++itr;
      eosio::check(itr.value().baz == 2, "Got the wrong value");
   }

   [[eosio::action]]
   void iterationi() {
      my_table t;

      auto begin_itr = t.ifoo_index.begin();
      auto end_itr = t.ifoo_index.end();

      // operator++ (case insensitive string)
      // ----------
      auto itr = t.ifoo_index.begin();
      eosio::check(itr != end_itr, "Should not be the end");
      eosio::check(itr.value().foo == "a", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "C", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "e", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "g", "Got the wrong value");
      ++itr;
      eosio::check(itr.value().foo == "I", "Got the wrong value");
      ++itr;
      eosio::check(itr == end_itr, "Should be the end");

      // operator-- (case sensitive string)
      // ----------
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr != begin_itr, "Should not be the beginning");
      --itr;
      eosio::check(itr == begin_itr, "Should be the beginning");
   }

   [[eosio::action]]
   void range() {
      my_table t;

      std::vector<my_struct> expected{s5, s4, s3};
      uint64_t b = 1;
      uint64_t e = 3;
      auto vals = t.bar_index.range(b, e);
      eosio::check(vals == expected, "range did not return expected vector");

      expected = {s3};
      vals = t.bar_index.range(e, e);
      eosio::check(vals == expected, "range did not return expected vector");
   }
};
