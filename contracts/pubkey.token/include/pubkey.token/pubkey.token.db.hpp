 #pragma once

#include <eosio/asset.hpp>
#include <eosio/privileged.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <utils.hpp>

#include <deque>
#include <optional>
#include <string>
#include <map>
#include <set>
#include <type_traits>

namespace flon {

using namespace std;
using namespace eosio;

#define SYMBOL(sym_code, precision) symbol(symbol_code(sym_code), precision)

static constexpr eosio::name active_perm{"active"_n};

static constexpr uint64_t percent_boost     = 10000;
static constexpr uint64_t max_memo_size     = 1024;
static constexpr uint64_t max_addr_len      = 128;

static constexpr symbol FLON_SYMBOL              = SYMBOL("FLON", 8);
static constexpr name  FLON_BANK                = "flon.token"_n;

#define hash(str) sha256(const_cast<char*>(str.c_str()), str.size())

enum class err: uint8_t {
    NONE                = 0,
    RECORD_NOT_FOUND    = 1,
    RECORD_EXISTING     = 2,
    ADDRESS_ILLEGAL     = 3,
    SYMBOL_MISMATCH     = 4,
    ADDRESS_MISMATCH    = 5,
    NOT_COMMON_XIN      = 6,
    STATUS_INCORRECT    = 7,
    PARAM_INCORRECT     = 8,
    NO_AUTH             = 9,

};

#define TBL struct [[eosio::table, eosio::contract("pubkey.token")]]

struct [[eosio::table("global"), eosio::contract("pubkey.token")]] global_t {
    name     admin         = "amaxapplybbp"_n;                 
    asset    total_claimed = asset(0, FLON_SYMBOL); 
    name     last_idx; 

    EOSLIB_SERIALIZE( global_t, (admin)(total_claimed)(last_idx)(bbp_count) )
};
typedef eosio::singleton< "global"_n, global_t > global_singleton;

TBL pubkey_t {
    uint64_t            id;
    eos::public_key     pubkey;
    asset               quant;
    time_point          last_transfer_at;

    pubkey_t() {};

    uint64_t    primary_key()const { return id; }
    
    eosio::checksum256 by_pubkey() const {
        return eosio::sha256(reinterpret_cast<const char*>(&pubkey), sizeof(pubkey));
    }

    typedef eosio::multi_index<"pubkeys"_n,  pubkey_t,
        indexed_by<"by.pubkey"_n, const_mem_fun<pubkey_t, checksum256, &pubkey_t::by_pubkey> >
    > idx_t;

    EOSLIB_SERIALIZE( pubkey_t, (id)(pubkey)(quant)(last_transfer_at) )
};

} // flon
