/*
 *  Copyright 2017 Zhenfei Zhang @ onboard security
 *
 *  This file is part of pqNTRUSign signature scheme with bimodal
 *  Gaussian sampler (Gaussian-pqNTRUSign).
 *
 *  This software is released under GPL:
 *  you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software
 *  Foundation, either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  You should have received a copy of the GNU General Public License.
 *  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "Gaussian-pqNTRUSign.h"
#include "param.h"
#include "poly/poly.h"


uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}


int main(void) {

    int64_t   *f, *g, *g_inv, *h, *buf, *msg, *sig;
    PQ_PARAM_SET *param = pq_get_param_set_by_id(Guassian_512_107);
    uint64_t startc, endc, signtime = 0, verifytime = 0;
    clock_t start, end;
    double cpu_time_used1;
    double cpu_time_used2;
    f = malloc ( sizeof(int64_t)*param->N);
    g = malloc ( sizeof(int64_t)*param->N);
    g_inv = malloc ( sizeof(int64_t)*param->N);
    h = malloc ( sizeof(int64_t)*param->N);
    buf = malloc ( sizeof(int64_t)*param->N*11);
    msg = malloc ( sizeof(int64_t)*param->N*2);
    sig = malloc ( sizeof(int64_t)*param->N);


    /* generate a set of keys */
    keygen(f,g,g_inv,h,buf,param);

    /* generate a message vector to sign */
    pol_gen_flat(msg, param->N, param->d);
    pol_gen_flat(msg+param->N, param->N, param->d);

    /* sign the msg */
    sign(sig, msg, f,g,g_inv,h,buf,param);

    /* verifying the signature */
    printf("%d \n", verify(sig, msg, h,buf,param));

    int i =0, j=0;
    int counter = 0;

    for (i=0;i<1000;i++)
    {

        binary_poly_gen(msg, param->N*2);
        /* sign the msg */
        start = clock();
        startc = rdtsc();
        counter += sign(sig, msg, f,g,g_inv,h,buf,param);
        endc = rdtsc();
        end = clock();
        cpu_time_used1 += (end-start);
        signtime += (endc-startc);
        /* verifying the signature */
        startc = rdtsc();
        start = clock();
        if(verify(sig, msg, h,buf,param)!=0)
            printf("%d error\n", i);
        end = clock();
        cpu_time_used2 += (end-start);
        endc = rdtsc();
        verifytime += (endc-startc);
    }
    printf("it takes %d samples to generate %d number of signatures!\n", counter, i);
    printf("average signing time: %f clock cycles or %f seconds!\n", (double)signtime/i, cpu_time_used1/i/CLOCKS_PER_SEC);
    printf("average verification time:  %f clock cycles or %f seconds!\n", (double)verifytime/i, cpu_time_used2/i/CLOCKS_PER_SEC);
    /* sign the msg */
    batch_sign(sig, msg, f,g,g_inv,h,buf,param);

    /* verifying the signature */
    printf("%d \n", batch_verify(sig, msg, h,buf,param));


    /* batch verification */
    int64_t *batchmsg, *batchsig;
    batchmsg = malloc ( sizeof(int64_t)*param->N*2);
    batchsig = malloc ( sizeof(int64_t)*param->N);
    memset(batchsig, 0, sizeof(int64_t)*param->N);
    memset(batchmsg, 0, sizeof(int64_t)*param->N*2);
    counter = 0;

    for (i=0;i<10000;i++)
    {

        binary_poly_gen(msg, param->N*2);

        /* sign the msg */
        counter += batch_sign(sig, msg, f,g,g_inv,h,buf,param);


        for (j=0;j<param->N*2;j++)
            batchmsg[j] = (batchmsg[j]+msg[j])%2;


        for (j=0;j<param->N;j++)
            batchsig[j] = batchsig[j]+sig[j];

        /* verifying the signature */
        if(batch_verify(batchsig, batchmsg, h,buf,param)!=0)
        {
            break;
        }
    }
    printf("it takes %d samples to generate %d number of signatures!\n", counter, i);
    printf("batch verifed for %d signatures!\n", i);

	return EXIT_SUCCESS;
}


