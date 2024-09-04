#!/bin/bash
cargo build -Z  build-std=core,alloc   --features env,net,thread,untrusted_time,untrusted_fs,unsupported_process,capi,backtrace  --target ../x86_64-unknown-linux-sgx.json
