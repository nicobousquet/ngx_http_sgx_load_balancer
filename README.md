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

## Details
- ### Linking warnings during enclave compilation
	``./compile`` compile separately the SGX enclave and nginx. This is because nginx manage the compilation of its modules by itself, and the enclave isn't included in the module. It is a separate shared object which will be loaded during runtime.

	Because of that, the enclave compilation will raise
	```
	g++: warning: * : linker input file unused because linking not done
	```
	Just ignore it.

-  ### the ``enclave.signed.so`` file
	Once nginx compiled, the ``./compile`` script will copy and paste the ``enclave.signed.so`` file (generated in the module directory) side by side with the ``nginx`` symbolic link at the root of this repo.
	It is needed to run ``nginx`` side by side with ``enclave.signed.so``. (Or using environnement variables)
