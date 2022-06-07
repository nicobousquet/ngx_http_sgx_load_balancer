/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>

# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;


int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;    
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        printf("sgx error %i\n", ret);
        return -1;
    }
    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str) {
    printf("%s", str);
}



///////////////////////////////////////////////////////
///////////////////////////////////////////////////////












#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <iostream>

char * ngx_http_upstream_custom_registration(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
ngx_int_t ngx_http_upstream_custom_init(ngx_conf_t *cf, ngx_http_upstream_srv_conf_t *us);
static ngx_int_t ngx_http_upstream_custom_init_peer(ngx_http_request_t *r, ngx_http_upstream_srv_conf_t *us);
static ngx_int_t ngx_http_upstream_get_custom_load_balancer_peer(ngx_peer_connection_t *pc, void *data);
void ngx_http_upstream_free_custom_load_balancer_peer(ngx_peer_connection_t *pc, void *data, ngx_uint_t state);


typedef struct {
    struct sockaddr * sockaddr;
    socklen_t socklen;
    ngx_str_t * name;
} peer_t;


typedef struct {
    int len;
    peer_t * peers;

} peers_list_t;


//context
static ngx_http_module_t  ngx_http_custom_load_balancer_module_ctx = { 
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


// Directive declaration
// https://www.nginx.com/resources/wiki/extending/api/configuration/
static ngx_command_t  ngx_http_custom_load_balancer_commands[] = { 
    { ngx_string("truc"),
      NGX_HTTP_UPS_CONF|NGX_CONF_NOARGS,
      ngx_http_upstream_custom_registration, // registration function
      0,
      0,
      NULL },

      ngx_null_command
};



// Module declaration
// https://www.nginx.com/resources/wiki/extending/api/main/#c-ngx-module-t
ngx_module_t ngx_http_custom_load_balancer_module = {
    NGX_MODULE_V1,
    &ngx_http_custom_load_balancer_module_ctx,      /* module context */
    ngx_http_custom_load_balancer_commands,  /* module directives */
    NGX_HTTP_MODULE,              /* module type */
    NULL,                         /* init master */
    NULL,                         /* init module */
    NULL,                         /* init process */
    NULL,                         /* init thread */
    NULL,                         /* exit thread */
    NULL,                         /* exit process */
    NULL,                         /* exit master */
    NGX_MODULE_V1_PADDING
};


//registration function
char * ngx_http_upstream_custom_registration(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    printf("registration function\n");
    std::cout << "test c++\n";


    /////////////////////////////////////
    if(initialize_enclave() < 0){
        printf("Failed initializing enclave\n");
    }
    
    int a;
    ecall_printf(global_eid, &a);
    printf("a=%i\n",a);

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    ////////////////////////////////////




    ngx_http_upstream_srv_conf_t  *uscf;

    uscf = (ngx_http_upstream_srv_conf_t *) ngx_http_conf_get_module_srv_conf(cf, ngx_http_upstream_module);

    /*
    if (uscf->peer.init_upstream) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                           "load balancing method redefined");
    }
    */

    uscf->peer.init_upstream = ngx_http_upstream_custom_init;
    uscf->flags = NGX_HTTP_UPSTREAM_CREATE
                  |NGX_HTTP_UPSTREAM_WEIGHT
                  |NGX_HTTP_UPSTREAM_MAX_CONNS
                  |NGX_HTTP_UPSTREAM_MAX_FAILS
                  |NGX_HTTP_UPSTREAM_FAIL_TIMEOUT
                  |NGX_HTTP_UPSTREAM_DOWN;

    return NGX_CONF_OK;
 }


//upstream initialization function
ngx_int_t ngx_http_upstream_custom_init(ngx_conf_t *cf, ngx_http_upstream_srv_conf_t *us) {
    printf("upstream initialization function\n");

    us->peer.init = ngx_http_upstream_custom_init_peer;


    ngx_uint_t                       i, j, n;
    ngx_http_upstream_server_t      *server;

    server = (ngx_http_upstream_server_t *) us->servers->elts;

    for (n = 0, i = 0; i < us->servers->nelts; i++) {
        n += server[i].naddrs;
    }

    peers_list_t * peers = (peers_list_t *) malloc(sizeof(peers_list_t));
    peers->len = n;
    peers->peers = (peer_t *) malloc(sizeof(peer_t) * n);

    printf("List of peers:\n");
    for (n = 0, i = 0; i < us->servers->nelts; i++) {
        for (j = 0; j < server[i].naddrs; j++, n++) {
            peers->peers[n].sockaddr = server[i].addrs[j].sockaddr;
            peers->peers[n].name = &server[i].addrs[j].name;

            struct sockaddr_in6 * endpoint = (struct sockaddr_in6 *) server[i].addrs[j].sockaddr;

            int ip_size;
            char ip[45] = { '\0' };
            if(endpoint->sin6_family == AF_INET6){
                ip_size = INET6_ADDRSTRLEN;
                peers->peers[n].socklen = sizeof(struct sockaddr_in6);
                inet_ntop(AF_INET6, &endpoint->sin6_addr, ip, ip_size);
            } else {
                ip_size = INET_ADDRSTRLEN;
                peers->peers[n].socklen = sizeof(struct sockaddr_in);
                inet_ntop(AF_INET, &((struct sockaddr_in *)endpoint)->sin_addr, ip, ip_size);
            }
            uint16_t port;
            port = htons (endpoint->sin6_port);
            
            
            printf("    ↳ Peer n°%i:\n",(int) n);
            printf("        ↳ ip: %s\n",ip);
            printf("        ↳ port: %i\n",port);
            printf("        ↳ name: %s\n",server[i].addrs[j].name.data);
        }
    }
    
    us->peer.data = peers;

    return NGX_OK;

}


//peer initialization function, choose the server
static ngx_int_t ngx_http_upstream_custom_init_peer(ngx_http_request_t *r, ngx_http_upstream_srv_conf_t *us) {
    printf("peer initialization function\n");

    peers_list_t * peers = (peers_list_t *) us->peer.data;

    r->upstream->peer.free = ngx_http_upstream_free_custom_load_balancer_peer;
    r->upstream->peer.get = ngx_http_upstream_get_custom_load_balancer_peer;
    r->upstream->peer.tries = 1;

    int random = rand()%peers->len;
    printf("=> Peer n°%i\n",random);
    r->upstream->peer.data = (void *) &peers->peers[random];

    //us->host.data; // "backend"
    return NGX_OK;
}


//redirection
static ngx_int_t ngx_http_upstream_get_custom_load_balancer_peer(ngx_peer_connection_t *pc, void *data){
    printf("redirect\n");

    peer_t * peer = (peer_t *) data;

    pc->sockaddr = peer->sockaddr;
    pc->socklen  = peer->socklen;
    pc->name     = peer->name;
    
    return NGX_OK;
}



void ngx_http_upstream_free_custom_load_balancer_peer(ngx_peer_connection_t *pc, void *data, ngx_uint_t state) {
    printf("free\n");
    //connection finished
    //update stats ?
    pc->tries = 0;
}


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////