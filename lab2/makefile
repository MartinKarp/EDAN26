NSYM	= 100
NVERTEX	= 10
MAXSUCC	= 4
NACTIVE	= 10
PRINT   = 1

N2SYM	= 10000
N2VERTEX	= 1000
M2AXSUCC	= 40
N2ACTIVE	= 100
P2RINT   = 0

all:
	@mkdir -p classes
	#scalac -g:vars -d classes dataflow.scala
	#scalac -deprecation -d classes dataflow.scala
	scalac -deprecation -optimise -d classes dataflow.scala
	scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(PRINT)
	scala -classpath classes Driver $(N2SYM) $(N2VERTEX) $(M2AXSUCC) $(N2ACTIVE) $(P2RINT)

run:
	scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(PRINT)
	scala -classpath classes Driver $(N2SYM) $(N2VERTEX) $(M2AXSUCC) $(N2ACTIVE) $(P2RINT)
