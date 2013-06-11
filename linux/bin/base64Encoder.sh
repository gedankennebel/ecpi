#!/bin/bash
user=$1
pass=$2

auth=$(echo -ne "$user:$pass" | base64);

echo $auth
