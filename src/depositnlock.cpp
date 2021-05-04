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
         s.deposit_tp = current_time_point(); // define as new time
      });
   }
   else
   {
      _deposits.emplace(get_self(), [&](auto &s) {
         s.owner = from;
         s.balance = quantity;
         s.deposit_tp = current_time_point();
      });
   }
}


ACTION depositnlock::set( name user, uint64_t value ) {
   check( has_auth(get_self()), "You need permission to set the singleton.");
   check( get_self() == user, "Only the contractor owner can set the singleton.");
   
   auto entry_stored = singleton_instance.get_or_create(user, settingsrow);
   entry_stored.primary_value = user;
   entry_stored.minutes_to_wait = value;
   singleton_instance.set(entry_stored, user);
}

ACTION depositnlock::get( ) {
   if (singleton_instance.exists())
      eosio::print(
         "Minutes to wait after depsoit to: ", 
         name{singleton_instance.get().primary_value.value},
         " is ",
         singleton_instance.get().minutes_to_wait,
         ", then you can withdraw.\n");
   else
      eosio::print("Singleton is empty\n");
}

uint64_t depositnlock::get_value( ){
   if (singleton_instance.exists())
      return singleton_instance.get().minutes_to_wait;
   else
      return 0; // no need to wait
} 
   

ACTION depositnlock::withdraw(name owner, asset quantity, std::string memo){
   check( has_auth(owner), "You need permission to withdraw.");
   check( memo.length() <= 128, "The length of memo greater than 128 chars.");
   
   //deposits _deposits( get_self(), owner.value );
   
   auto itr = _deposits.find( owner.value );
   check(itr != _deposits.end(), "Are you sure you have deposit with us?");
   check( itr->balance >= quantity, " You seems to withdraw too much. ");

   uint64_t m = get_value();
   eosio::print("time to wait is ", m, " minute(s).\n");
   check( (itr->deposit_tp + (time_point)minutes(m) ) < current_time_point() ,"you can withdraw only after minimum stake time.");

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
      name("tokenfactory"),
      "transfer"_n,
      std::make_tuple( get_self(), owner, quantity, memo )
   }.send();

}