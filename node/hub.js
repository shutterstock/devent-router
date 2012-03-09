var zmq = require('zmq')
	, nomnom = require('nomnom')
	, util = require('util');

var opts = nomnom
	.opts({
		'pull': {
			string: '--pull',
			default: 'tcp://127.0.0.1:6666'
		},
		'pub': {
			string: '--pub',
			default: 'tcp://127.0.0.1:7777'
		},
	}).parseArgs();

var puller = zmq.socket('pull');
puller.bindSync(opts.pull);

var publisher = zmq.socket('pub');
publisher.bindSync(opts.pub);

puller.on('message', function() {
	for (var i = 0; i < arguments.length; i++) {
		publisher.send(
			arguments[i], 
			(i + 1 < arguments.length ? zmq.ZMQ_SNDMORE : undefined)
		);
		
	}
}); 

puller.on('error', function(err) {
	util.log("pull error: " + err.toString())
});
publisher.on('error', function(err) {
	util.log("pub error: " + err.toString())
});
