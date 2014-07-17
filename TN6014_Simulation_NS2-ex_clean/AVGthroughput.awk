#ex1.tcl

BEGIN {
	recvdSize = 0
	startTime = 1e6
	stopTime = 0
}

{
	event = $1
	time = $2
	node_s = $3
	node_r = $4
	type = $5
	pkt_size = $6
	flags = $7
	flowid = $8
	source = $9
	destination = $10
	seqno = $11
	pktid = $12
		   
	# Store start time
	if (event == "+" && type == "tcp") {
		if (time < startTime) {
			startTime = time
		}
	}
   
	# Update total received packets' size and store packets arrival time
	if (event == "r" && type == "tcp" && node_r == 2) {
		if (time > stopTime) {
			stopTime = time
		}
		# Rip off the header
		hdr_size = 40
		pkt_size -= hdr_size
		# Store received packet's size
		recvdSize += pkt_size
		#thrp-vs-time
		#printf("%f %f\n",time,(recvdSize/(stopTime-startTime))*(8/1000))
	}
}
   
END {
	printf("Average Throughput[kbps] = %.2f\t\t StartTime=%.2f\tStopTime=%.2f\n",(recvdSize/(stopTime-startTime))*(8/1000),startTime,stopTime)
}

