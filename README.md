oplog
=====

oplog is a collection of log aggregation tools.

### Getting Started

First build oplog:

    make

Next start `oplog-hub` and `oplog-recv`:

    ./oplog-hub 'tcp://*:5555' 'tcp://*:5556' &
    ./oplog-recv 'tcp://*:5556'

In a new window open another instance of `oplog-recv`:

    ./oplog-recv 'tcp://*:5556'

And in one more window run `oplog-send`:

    echo "hello world" | ./oplog-send 'tcp://*:5555'

### Requirements

* [libzmq](https://github.com/zeromq/libzmq) >= 2.1

### License

This work is licensed under the MIT License (see the LICENSE file).
