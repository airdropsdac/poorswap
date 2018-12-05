#include <eosiolib/eosio.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;
using std::string;

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



    typedef multi_index<N(claimables), CycleData>    Claimables;
    typedef singleton<  N(cycles),     CycleData>    Cycles;
    typedef singleton<  N(settings),   settings>     Settings;




    int64_t getCurrentCycle(){
        int64_t startTime = Settings(_self, N("started")).get().setting;
        return ((now() - startTime) / 3600) / 24;
    }

    CycleData getCycleData(int64_t cycle){
        Cycles cycles(_self, cycle);
        return cycles.get_or_default(CycleData{cycle, asset(0'0000, string_to_symbol(4, "POOR"))});
    }

    void setCycleData(int64_t cycle, asset quantity){
        Cycles cycles(_self, cycle);
        CycleData cycleData = getCycleData(cycle);
        cycleData.tokens = quantity;
        cycles.set(cycleData, _self);
    }

public:
    using contract::contract;
    scatterfunds( name self ) : contract(self){}

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

    void buy( const currency::transfer& t ){
      if( t.to == _self ) {
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "POOR"), "Token must be POOR");
        eosio_assert(t.quantity.is_valid(), "Token asset is not valid");
        eosio_assert(t.quantity.amount >= 1'0000, "Not enough tokens");

        int64_t cycle(getCurrentCycle());
        eosio_assert(cycle <= lastCycle, "3 years have passed, this development fundraiser is over.");

        CycleData cycleData = getCycleData(cycle);
        asset total = cycleData.tokens;
        eosio_assert(total < maxEosPerCycle(), "Too much has been spent today");

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
            buy( unpack_action_data<eosio::currency::transfer>() );
            return;
        }

        if( action == N(buy) ){
            eosio_assert(false, "Can't call buy directly");
        }

        if( contract != _self ) return;
        auto& thiscontract = *this;
        switch( action ) {
            EOSIO_API( scatterfunds, (start)(claim) )
        };
    }



};

extern "C" {
    [[noreturn]] void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        scatterfunds c( receiver );
        c.apply( code, action );
        eosio_exit(0);
    }
}