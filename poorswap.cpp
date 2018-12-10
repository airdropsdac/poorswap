#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;
using std::string;

//THANKS TO NSJAMES FOR THE ORIGINAL OPENSOURCE CODE
//YOU THE BEST MAN AND I LOVE YOU <3

class poorswap : contract {
private:

    // 2 years ( 1095 days )
    static const int64_t lastCycle = 1095;

    asset tokensPerCycle(){
        return asset(3331, string_to_symbol(0, "ZKS"));
    }

    // asset maxEosPerCycle(){
    //     return asset(2000'0000, string_to_symbol(4, "EOS"));
    // }

    // @abi table claimables
    // @abi table cycles
    struct CycleData {
        int64_t   cycle;
        asset     tokens;

        int64_t primary_key() const { return cycle; }
        EOSLIB_SERIALIZE( CycleData, (cycle)(tokens) )
    };

    // @abi table settings
    struct settings {
        int64_t   setting;

        int64_t primary_key() const { return setting; }
        EOSLIB_SERIALIZE( settings, (setting) )
    };

    struct transfer_args {
            account_name  from;
            account_name  to;
            asset         quantity;
            string        memo;
         };



    typedef multi_index<N(claimables), CycleData>    Claimables;
    typedef multi_index<N(cycles),     CycleData>    Cycles;
    typedef singleton<  N(settings),   settings>     Settings;



    int64_t getCurrentCycle(){
        int64_t startTime = Settings(_self, N("started")).get().setting;
        return ((now() - startTime) / (24*3600)); //24*3600 for prod, 180 for testing (2 minutes)
    }

    CycleData getCycleData(int64_t cycle){
        Cycles cycles(_self, _self);
        auto itr = cycles.find(cycle);
        if(itr == cycles.end()) {
          return CycleData{cycle, asset(0'0000, string_to_symbol(4, "POOR"))};
        } else {
          return CycleData{itr->cycle, itr->tokens};
        }
    }

    void setCycleData(int64_t cycle, asset quantity) {
        Cycles cycles(_self, _self);
        auto itr = cycles.find(cycle);
        if(itr == cycles.end()) {
          cycles.emplace(_self, [&](auto& row){
            row.cycle = cycle;
            row.tokens = quantity;
          });
        } else {
          cycles.modify(itr, 0, [&](auto& row){
            row.tokens = quantity;
          });
        }
    }

public:
    using contract::contract;
    poorswap( name self ) : contract(self){}

    // @abi action
    void start(){
        require_auth(_self);
        eosio_assert(!Settings(_self, N("started")).exists(), "Already Started");
        Settings(_self, N("started")).set(settings{now()}, _self);
    }

    // @abi actions
    void claim( account_name owner ){
        Claimables claimables(_self, owner);
        eosio_assert(claimables.begin() != claimables.end(), "Account has nothing to claim");

        asset dispensable(0, string_to_symbol(0, "ZKS"));

        uint64_t currentCycle = getCurrentCycle();
        asset perCycle = tokensPerCycle();


        auto claimable = claimables.begin();
        while(claimable != claimables.end()){
            if(claimable->cycle < currentCycle){
                asset total = getCycleData(claimable->cycle).tokens;
                float percentage = (float)claimable->tokens.amount / total.amount;
                dispensable += asset(perCycle.amount*percentage, string_to_symbol(0, "ZKS"));
                claimables.erase(claimable);
                claimable = claimables.begin();
            } else claimable++;
        }

        eosio_assert(dispensable.amount > 0, "Not ready to dispense yet.");

        action(
            permission_level{ _self, N(claims) },
            N(zkstokensr4u), N(transfer),
            std::make_tuple(_self, owner, dispensable, std::string("ZKS Claimed"))
        ).send();
    }





    /**********************************************/
    /***                                        ***/
    /***             Token Transfers            ***/
    /***                                        ***/
    /**********************************************/

    void buy( const transfer_args& t ){
      if( t.to == _self ) {
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "POOR"), "Token must be POOR");
        eosio_assert(t.quantity.is_valid(), "Token asset is not valid");
        eosio_assert(t.quantity.amount >= 1'0000, "Not enough tokens");

        int64_t cycle(getCurrentCycle());
        eosio_assert(cycle <= lastCycle, "3 years have passed, this promotion fundraiser is over.");

        CycleData cycleData = getCycleData(cycle);
        asset total = cycleData.tokens;

        asset quantity = t.quantity;
        setCycleData(cycle, total + quantity);

        Claimables claimables(_self, t.from);
        auto iter = claimables.find(cycle);
        if(iter == claimables.end()) claimables.emplace(_self, [&](auto& row){
                row.cycle = cycle;
                row.tokens = quantity;
            });
        else claimables.modify(iter, 0, [&](auto& row){
                row.tokens += quantity;
            });
      }
    }

    void apply( account_name contract, account_name action ) {
        if( contract == N(poormantoken) && action == N(transfer) ) {
            buy( unpack_action_data<transfer_args>() );
            return;
        }

        if( action == N(buy) ){
            eosio_assert(false, "Can't call buy directly");
        }

        if( contract != _self ) return;
        auto& thiscontract = *this;
        switch( action ) {
            EOSIO_API( poorswap, (start)(claim) )
        };
    }

};

extern "C" {
    [[noreturn]] void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        poorswap c( receiver );
        c.apply( code, action );
        eosio_exit(0);
    }
}
