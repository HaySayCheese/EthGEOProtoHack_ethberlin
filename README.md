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
* Check `bin/` directory; 

</br>

### How to run

##### UUID2Address
UUID2Address is used for addresation purposes in test net. evironments.  
It requires `redis-server` to be up and running on `127.0.0.1:6379`.  
It also requires `cfg.json` with the configuration in the same directory (sample config).



##### Simple config
