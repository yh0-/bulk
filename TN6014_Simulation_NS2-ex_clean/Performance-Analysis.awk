# ex4.tcl,wrls.tcl

BEGIN {
	recvdSize = 0
	currTime = prevTime = 0
	startTime = 1e6
	stopTime = 0
	pktsent =0
	pktrecv =0
	avg_hops = 0
	h_pktsent =0
	hops =0
	GPSR_beacon=0
	#printf("# %10s %10s %5s %5s %15s %18s\n\n", \
	 #      "flow","flowType","src","dst","time","throughput")
}

{
	# Trace line format: normal
		#event = $1
		#time = $2
		#if (event == "r" || event == "d") 
		#node_id = $3
		#flow_id =$14 
		#pkt_id = $6
		#pkt_size = $8
		#flow_t = $7
		#level = $4

	#Computing Control Overhead
	if ($4 == "RTR" && $7 == "AODV"  &&  $1 == "s" ) {
		RTRmsg += 1
	}


	#Computing e2eDelay - Throughput - Loss

	# Store packets send time
	if ($4 == "AGT" && $7 == "tcp" &&  $1 == "s" ) {
		if ($2 < startTime) {
			startTime = $2
		}
		sendTime[$6] = $2
		pktsent += 1
		#printf("%f\n",$2, $6)
	}

	# Update total received packets' size and store packets arrival time
	if ($4 == "AGT" && $7 == "tcp" &&  $1 == "r" ) {
		if ($2 > stopTime) {
			stopTime = $2
		}
		# Store packet's reception time
		recvTime[$6] = $2

		# Store received packet's size
		recvdSize += $8

		#currTime = (time - prevTime)
		pktrecv += 1
	}
}
END {
	# Compute average delay
	delay = avg_delay = recvdNum = 0
	for (i in recvTime) {
		if (sendTime[i] < recvTime[i]) {
		#printf("%d\t %f\n",i,recvTime[i] - sendTime[i]);
		delay += recvTime[i] - sendTime[i]
		recvdNum ++
		}
	}
	if (recvdNum != 0) {
		avg_delay = delay / recvdNum
	} else {
		avg_delay = 1
	}
	# computing throughput
	if (stopTime-startTime != 0) {
		thrp=(recvdSize/(stopTime-startTime))*(8/1000)
	} 

	#computing packet delivery ratio
	pktDR = 0
	if (pktsent > 0 && pktsent >= pktrecv) {
		pktDR=pktrecv/pktsent*100
	}

	# compute pkt loss ratio
	pktloss = pktlossratio = 0

	if (pktsent >0 && pktsent >pktrecv) {
		pktloss = pktsent - pktrecv
		pktlossratio = pktloss/pktsent*100
	}
	printf("%d %7s %.2f %6s %.2f %6s %.2f %7s %.2f \n",x , "T[k]=", thrp, "L[%]=", pktlossratio, "D[ms]=", avg_delay*1000, "PDR=",pktDR)
}
