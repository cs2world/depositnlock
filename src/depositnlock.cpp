#include <depositnlock.hpp>


void depositnlock::deposit(name from, name to, asset quantity, std::string memo)
{
   if (from == _self || to != _self)
      return;

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