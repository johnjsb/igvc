import serial, json
import time
import atexit

SERIAL_LINE = "COM6"
BAUD_RATE = 115200

def printError(s):
    print "ERROR: ", s

def onClose():
    connection.close()

atexit.register(onClose)

# initialize connection
connection = serial.Serial(
    port = SERIAL_LINE, 
    baudrate = BAUD_RATE, 
    timeout = .2, 
    writeTimeout = 1
    )

def processData(data):
    try:
        count = int(data["SPLMdebug"])
    except:
        printError("couldn't receive values for expected keys") 
        return	
        
    print "counter received: %d" % count

def parseLine(s):
    try :
        data = json.loads(s.strip())
    except:
        print "ERROR: problem while parsing JSON: ", s.strip()
        return

    processData(data)

counter = 0

connection.flush()

while True:
	try:
		# send message
		msg = json.dumps({"SPLM":str(counter)})
		print "sent: %s" % msg
		
		connection.write(msg + '\n')
		connection.flushOutput()

		# read response
		connection.flushInput()
		line = connection.readline()

		if len(line) == 0:
			printError("ERROR: timeout")
		else:
			print line
			parseLine(line)

		counter += 1
		time.sleep(1)
	except: 
		break

connection.close()
