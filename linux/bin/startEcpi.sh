#!/bin/bash
user=$1
pass=$2
ip = $3

auth=$(echo -ne "$user:$pass" | base64);

sudo ./ecpi ip auth

