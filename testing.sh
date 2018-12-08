cleos create account eosio zkstokensr4u EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio poormantoken EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio zks4poorswap EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV


cleos set contract poormantoken . token.wasm token.abi
cleos set contract zkstokensr4u . token.wasm token.abi
cleos set contract zks4poorswap . poorswap.wasm poorswap.abi

cleos push action zkstokensr4u create '["zkstokensr4u","100000000 ZKS"]' -p zkstokensr4u
cleos push action poormantoken create '["poormantoken","100000000.0000 POOR"]' -p poormantoken

cleos set account permission zks4poorswap claims '{"threshold":1,"accounts":[{"permission":{"actor":"zks4poorswap","permission":"eosio.code"},"weight":1}]}' active -p zks4poorswap
cleos set action permission zks4poorswap zkstokensr4u transfer claims


cleos push action poormantoken issue '["user1","10000.0000 POOR","Init"]' -p poormantoken
cleos push action poormantoken issue '["user2","10000.0000 POOR","Init"]' -p poormantoken
cleos push action poormantoken issue '["user3","10000.0000 POOR","Init"]' -p poormantoken

cleos push action zkstokensr4u issue '["zks4poorswap","3000000 ZKS","Init"]' -p zkstokensr4u

cleos push action zks4poorswap start '[]' -p zks4poorswap

cleos push action poormantoken transfer '["user1","zks4poorswap","10.5151 POOR","Getr ZKS"]' -p user1
cleos push action poormantoken transfer '["user2","zks4poorswap","20.3232 POOR","Getr ZKS"]' -p user2
cleos push action poormantoken transfer '["user3","zks4poorswap","30.1234 POOR","Getr ZKS"]' -p user3

cleos get table zks4poorswap 0 cycles

cleos push action zks4poorswap claim '["user1"]' -p user1
cleos push action zks4poorswap claim '["user2"]' -p user2
cleos push action zks4poorswap claim '["user3"]' -p user3
