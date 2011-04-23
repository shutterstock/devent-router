zlog
====

zlog is a collection of log aggregation tools.

### Getting Started

First build zlog:

    make

Next start `zlog-hub` and `zlog-recv`:

    ./zlog-hub 'tcp://*:5555' 'tcp://*:5556' &
    ./zlog-recv 'tcp://*:5556'

In a new window open another instance of `zlog-recv`:

    ./zlog-recv 'tcp://*:5556'

And in one more window run `zlog-send`:

    echo "hello world" | ./zlog-send 'tcp://*:5555'

### Requirements

* [libzmq](https://github.com/zeromq/libzmq) >= 2.1

### License

This work is licensed under the MIT License (see the LICENSE file).
