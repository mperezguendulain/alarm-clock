/**
*	Autores : 	Martín Alejandro Pérez Güendulain
*				Angel Roberto Montez Murgas
*/

var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

const SerialPort = require('serialport');
const parsers = SerialPort.parsers;

const parser = new parsers.Readline({
	delimiter: '\r\n'
});

const port = new SerialPort('COM24', {
	baudRate: 9600
});

port.pipe(parser);

port.on('open', () => console.log('Port open'));

app.use(express.static('public'));

io.on('connection', function(socket){
	console.log('a user connected');
	socket.on('setTime', function(time) {
		console.log(time);
		port.write(time);
	});
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});
