cleos -u https://eos.greymass.com set contract zks4poorswap . poorswap.wasm poorswap.abi
cleos -u https://eos.greymass.com  set account permission zks4poorswap claims '{"threshold":1,"accounts":[{"permission":{"actor":"zks4poorswap","permission":"eosio.code"},"weight":1}]}' active -p zks4poorswap
cleos -u https://eos.greymass.com  set action permission zks4poorswap zkstokensr4u transfer claims
