#include <pubkey.token/pubkey.token.hpp>

#include <eosio/transaction.hpp>
#include<pubkey.token/flon.system.hpp>
#include<utils.hpp>
#include<math.hpp>
#include<string>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>


namespace flon {

//memo: 公钥
void pubkey_token::onrecv_token( name from, name to, asset quantity, string memo ){
   if (to != _self) return;
   if (from == _self) return;
   if (from == FLON_BANK) return;
   if (quantity.symbol != FLON_SYMBOL) return;
   if (memo.size() > max_memo_size) return;

   public_key pubkey = eosio::string_to_public_key(pubkey_str);

   _on_recv_quant(pubkey, quantity);

}

void pubkey_token::_on_recv_quant(const public_key& pubkey, const asset& quant) {
    // 获取二级索引
   auto idx = _pubkey_t.get_index<"by.pubkey"_n>();
   auto pubkey_hash = eosio::sha256(reinterpret_cast<const char*>(&pubkey), sizeof(pubkey));
   auto it = idx.find(pubkey_hash);

   // 检查公钥是否存在
   check(it != idx.end(), "Public key not found");
   if(it == idx.end()) {
      _pubkey_t.emplace(_self, [&](auto& row) {
         _gstate.last_idx++;
         row.id      = _gstate.last_idx;
         row.pubkey  = pubkey;
         row.quant   = quant;
         row.last_transfer_at = current_time_point();
      });
   } else {
      _pubkey_t.modify(*it, same_payer, [&](auto& row) {
         row.quant += quant;
         row.last_transfer_at = current_time_point();
      });
   }

}

void pubkey_token::newaccount(const eosio::public_key& pubkey, const name& acct, const eosio::signature& sig) {
   require_auth(acct);
   check(is_account(acct), "Account does not exist");
   check(pubkey != public_key(), "Invalid public key");
   check(sig != signature(), "Invalid signature");

   // 检查公钥是否存在
   auto idx = _pubkey_t.get_index<"by.pubkey"_n>();
   auto pubkey_hash = eosio::sha256(reinterpret_cast<const char*>(&pubkey), sizeof(pubkey));
   auto it = idx.find(pubkey_hash);
   check(it != idx.end(), "Public key not found");

   // 检查签名是否有效
   auto digest = hashname(acct);
   assert_recover_key(digest, sig, pubkey);

   // 创建账户
   action(
      permission_level{ _self, "active"_n },
      "eosio"_n, "newaccount"_n,
      std::make_tuple(_self, acct, pubkey, pubkey)
   ).send();

   //删除
   idx.erase(it);
}

void pubkey_token::move(const eosio::public_key& pubkey, time_point last_transfer_at, const name& to_acct, const eosio::signature& sig) {
   check(is_account(to_acct), "Account does not exist");
   check(pubkey != public_key(), "Invalid public key");
   check(sig != signature(), "Invalid signature");

   // 检查公钥是否存在
   auto idx = _pubkey_t.get_index<"by.pubkey"_n>();
   auto pubkey_hash = eosio::sha256(reinterpret_cast<const char*>(&pubkey), sizeof(pubkey));
   auto it = idx.find(pubkey_hash);
   check(it != idx.end(), "Public key not found");

   // 检查签名是否有效
   auto digest = hashtp(last_transfer_at);
   assert_recover_key(digest, sig, pubkey);

   // 检查最后转账时间
   check(it->last_transfer_at == last_transfer_at, "Invalid last transfer time");

   // 转账
   action(
      permission_level{ _self, "active"_n },
      "eosio.token"_n, "transfer"_n,
      std::make_tuple(_self, to_acct, it->quant, std::string("Transfer by public key"))
   ).send();

   // 删除
   idx.erase(it);
}

} // namespace flon



} //namespace flon

