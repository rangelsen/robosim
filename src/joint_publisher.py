import zmq
import time

if __name__ == "__main__":

	print("Joint publisher")
	
	ctx = zmq.Context()
	socket = ctx.socket(zmq.PUB)
	# socket.bind("ipc:///tmp/socket")
	socket.bind("tcp://127.0.0.1:5555")

	while True:
		msg = "joint 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0"
		socket.send(msg.encode())
		print("sent msg:", msg) 
		time.sleep(1.0)
