#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>
#include <wasm_db.hpp>
#include "pubkey.token/flon.token.hpp"
#include "pubkey.token/pubkey.token.db.hpp"

namespace flon {

using std::string;
using namespace eosio;
using namespace wasm::db;

/**
 * The `pubkey.token` is Cross-chain (X -> flon -> Y) contract
 * 
 */

#define TRANSFER(bank, from, to, quantity, memo) \
    {	token::transfer_action act{ bank, { {_self, active_perm} } };\
			act.send( from, to, quantity , memo );}


class [[eosio::contract("pubkey.token")]] pubkey.token : public contract {
private:
   dbc                  _db;
   global_singleton     _global;
   global_t             _gstate;
   pubkey_t::idx_t      _pubkey_t;
public:
   using contract::contract;

   pubkey_token(eosio::name receiver, eosio::name code, datastream<const char*> ds):
        _db(_self), contract(receiver, code, ds), 
        _pubkey_t(get_self(), get_self().value),
        _global(_self, _self.value){
            
        if (_global.exists()) {
            _gstate = _global.get();

        } else { // first init
            _gstate = global_t{};
            _gstate.admin = _self;
        }
    }

    ~pubkey_token() { 
        _global.set( _gstate, get_self() );
    }

    [[eosio::on_notify("*::transfer")]]
    void onrecv_token( name from, name to, asset quantity, string memo );
    

    //sign acct string 
    ACTION newaccount(const eosio::public_key& pubkey, const name& acct, const eosio::signature& sig);
    
    ACTION move(const eosio::public_key& pubkey, time_point last_transfer_at, const name& to_acct, const eosio::signature& sig);

    ACTION init( const name& admin) {
        _check_admin( );
        _gstate.admin  = admin;
    }

    private:
    void _check_admin(){
        CHECKC( has_auth(_self) || has_auth(_gstate.admin), err::NO_AUTH, "no auth for operate" )
    }

    void _on_recv_quant(const public_key& pubkey, const asset& quant)

};
} //namespace apollo
