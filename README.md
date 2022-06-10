# ngx_http_sgx_load_balancer
**ngx_http_sgx_load_balancer** is a load-balancing module for _**NGINX**_, that uses _**Intel SGX technology**_


## Getting started
First, setup the environnement:
 ```
 ./setup_nginx_env
 ```


Then configure nginx Makefiles with:
```
./custom_configure
```

Then compile both the module and nginx with:
```
./compile
```

Finally, nginx is ready to be run through ``./nginx``


## Requirements
Intel SGX SDK should be installed in ``/opt/intel/sgxsdk``.
