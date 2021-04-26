#include <depositnlock.hpp>

ACTION depositnlock::undeposit(name user, asset quantity)
{
   require_auth(user);

   auto itr = _deposits.require_find(user.value, "deposit not found");
   check(quantity <= itr->balance, "invalid quantity");

   _deposits.modify(itr, same_payer, [&](auto &s) {
      s.balance -= quantity;
   });

   if (itr->balance.amount == 0)
   {
      _deposits.erase(itr);
   }

   
   action{permission_level{get_self(), name("active")}, name("tokenfactory"), name("transfer"),
          make_tuple(get_self(), user, quantity, std::string("deposit withdraw..."))}
       .send();
}

ACTION depositnlock::withdraw(name user, uint64_t id)
{
   // require_auth(user);

   withdraws _withdraws(get_self(), user.value);

   auto req = _withdraws.require_find(id, "refund request not found");

   check(req->request_time + seconds(refund_delay_sec) <= current_time_point(),
         "refund is not available yet");

   action{permission_level{get_self(), name("active") }, name("tokenfactory"), name("transfer"),
          make_tuple(get_self(), user, req->withdraw_amount, std::string("deposit refund"))}
       .send();

   _withdraws.erase(req);
}

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