# ex1.tcl

BEGIN {
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
	if (event == "+" && type == "tcp" && node_s == 0) {
		send[pktid] = time
	}
   
	# Store recv time
	if (event == "r" && type == "tcp" && node_r == 2) {
		recv[pktid] = time
	}
}
   
END { 
	# Compute average delay
	for (i in recv) {
		if (recv[i] > send[i]) {
			delay += recv[i] - send[i]
			num++
		}
	}

	if (num != 0) {
		avg_delay = delay / num
	} else {
		avg_delay = 0
	}
	printf("\n   Average delay = %10g sec\n", avg_delay)
}

