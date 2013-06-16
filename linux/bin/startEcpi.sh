#!/bin/bash
ip=$1
port=$2
user=$3
pass=$4

auth=$(echo -ne "$user:$pass" | base64);

sudo ./ecpi $ip $port $auth

