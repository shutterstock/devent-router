devent-router
=============

devent-router is a collection of event aggregation tools.

### Getting Started

First build devent-router:

    make

Next start `dr-hub` and `dr-recv`:

    ./dr-hub 'tcp://*:5555' 'tcp://*:5556' &
    ./dr-recv 'tcp://*:5556' mytopic

In a new window open another instance of `dr-recv`:

    ./dr-recv 'tcp://*:5556' mytopic.second

And in one more window run `dr-send`:

    echo "both topics" | ./dr-send 'tcp://*:5555' mytopic
    echo "only second topic" | ./dr-send 'tcp://*:5555' mytopic.second

### Requirements

* [libzmq](https://github.com/zeromq/libzmq) >= 2.1

### License

[MIT](LICENSE) © 2011-2017 Shutterstock Images, LLC
