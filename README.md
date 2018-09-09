![](https://docs.google.com/drawings/d/e/2PACX-1vRJbTkJCCEdri8snzD7AUqWF7nGh7F8ymJsr37Ddnem9CEB1KSEM9QIS4K4ELhbRtR8ZLa4pdj9PK05/pub?w=1089&h=362)
</br>
</br>
</br>

# Hacked GEO Node 
Improved with [Ethereum Middleware Connector](https://github.com/HaySayCheese/EthSC_GEO_ETHBerlin)

</br>

### What is it?
It is a forked repo of the [original GEO Protocol node](https://github.com/GEO-Protocol/GEO-network-client) 
([Original Specs](https://github.com/GEO-Protocol/specs-protocol))

</br>

### How to build?
* Clone the repo;
* Ensure `boost-dev` and `libsodium` are present in your environment;
* Use `cmake` to build the project;
* Check `bin/` directory in the repo dir (usually is excluded from the .git); 

</br>

### How to run

##### UUID2Address
UUID2Address is used for addresation purposes in test net. evironments.  
It requires `redis-server` to be up and running on `127.0.0.1:6379`.  
It also requires `cfg.json` with the configuration in the same directory ([sample config](https://github.com/HaySayCheese/EthGEOProtoHack_ethberlin/blob/hackaton/uuid2address.cfg.example)).

##### Steps for node launching
1. Ensure uuid2address is up and running
2. Enshure node config is set up correctly and it is configured to use right uuid2address instance.
3. Run the node somelike in this way `./geo_engine`
4. Wait until it would be initialised properly
5. Begin transfer commands to the node.


##### Where are the commands located?
Commands are located [here](https://github.com/HaySayCheese/EthGEOProtoHack_ethberlin/tree/hackaton/src/core/interface/commands_interface/commands) and must be transferred via `commands.fifo` file. Results must be read via `results.fifo` in the same dir. At one moment of time only one result might be present in the `results.fifo` so to prevent node blocking it must be read ASAP.
