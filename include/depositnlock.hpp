#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;

CONTRACT depositnlock : public contract
{
public:
   using contract::contract;

   static constexpr symbol ant_symbol = symbol("DFS", 2);

   static constexpr uint32_t seconds_per_day = 1;
   static constexpr uint32_t refund_delay_sec = 3 * seconds_per_day;

   depositnlock(name receiver, name code, datastream<const char *> ds)
       : contract(receiver, code, ds),
         _deposits(get_self(), get_self().value)
   {
   }

   ACTION undeposit(name user, asset quantity);

   [[eosio::on_notify("tokenfactory::transfer")]] 
   void deposit(name from, name to, asset quantity, std::string memo);

   ACTION withdraw(name user, uint64_t id);

private:
   TABLE deposit_st
   {
      name owner;
      asset balance;

      uint64_t primary_key() const { return owner.value; }
   };

   TABLE withdraw_st
   {
      uint64_t id;
      asset withdraw_amount;
      time_point_sec request_time;
      uint64_t primary_key() const { return id; }
   };

   typedef multi_index<name("deposits"), deposit_st> deposits;
   deposits _deposits;

   typedef multi_index<name("withdraws"), withdraw_st> withdraws;
};