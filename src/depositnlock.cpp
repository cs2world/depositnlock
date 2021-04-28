#include <depositnlock.hpp>


void depositnlock::deposit(name from, name to, asset quantity, std::string memo)
{
   if ( from == get_self() || to != get_self() ) // avoiding when depositnlock transfer to others action.
      return;

   //deposits _deposits( get_self(), from.value );

   auto itr = _deposits.find(from.value);

   if (itr != _deposits.end())
   {
      _deposits.modify(itr, same_payer, [&](auto &s) {
         s.balance += quantity;
      });
   }
   else
   {
      _deposits.emplace(get_self(), [&](auto &s) {
         s.owner = from;
         s.balance = quantity;
      });
   }

}

ACTION depositnlock::withdraw(name owner, asset quantity, std::string memo){
   check( has_auth(owner), "You need permission to withdraw.");
   check( memo.length() <= 128, "The length of memo greater than 128 chars.");
   
   //deposits _deposits( get_self(), owner.value );
   
   auto itr = _deposits.find( owner.value );
   check(itr != _deposits.end(), "Are you sure you have deposit with us?");
   check( itr->balance >= quantity, " You seems to withdraw too much. ");

   if( itr->balance == quantity ) { // balance is 0 ***
      _deposits.erase( itr ); // remove the 0 balance records
   }
   else {   // not 0 ***, update it.
      _deposits.modify(itr, same_payer, [&](auto &s) {
         s.balance -= quantity;
      });
   }

   

   memo = std::string("Withdraw from 'depositnlock' with memo=[{ ") + memo + std::string(" }]");
   // memo.length could not > 256 chars
   
   action{
      permission_level{ get_self(), "active"_n },
      "tokenfactory"_n,
      "transfer"_n,
      std::make_tuple( get_self(), owner, quantity, memo )
   }.send();

}