#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;


CONTRACT depositnlock : public contract
{
public:
   using contract::contract;

   // This the constructor and all multi-index tables are instanciated.
   depositnlock(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds),
      _deposits( receiver, receiver.value ),
      singleton_instance(receiver, receiver.value)
      {  }

   
   [[eosio::on_notify("tokenfactory::transfer")]] 
   void deposit(name from, name to, asset quantity, std::string memo);

   ACTION withdraw(name onwer, asset quantity, std::string memo);
   
   ACTION set( name user, uint64_t value );
   ACTION get( );
   uint64_t get_value( );

private:
   TABLE testtable {
      name primary_value;
      uint64_t minutes_to_wait;
   } testtablerow;

   using singleton_type = eosio::singleton<"testtable"_n, testtable>;
   singleton_type singleton_instance;

   TABLE deposit_st
   {
      name owner;
      asset balance;
      eosio::time_point deposit_tp;

      uint64_t primary_key() const { return owner.value; }
   };

   typedef multi_index<name("deposits"), deposit_st> deposits;
   deposits _deposits;  // has issue with erase operation, No!

};