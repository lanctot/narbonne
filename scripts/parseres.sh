#!/bin/bash

bots[0]="exp"
bots[1]="simple"
lastindex=1

wins[0]=0
wins[1]=0

#group 1
#bots1[0]="star1"
#bots1[1]="star2"
#group 2
#bots2[0]="star1ss"
#bots2[1]="star2ss"
#lastindexg1=1
#lastindexg2=1

mco=0           # ss max chance outcomes

# the number of games will actually be double this
# because player positions are swapped
gamespermatch=1


for i in `seq 0 $lastindex`
do
  sj=`expr $i + 1`
  for j in `seq $sj $lastindex`
  do
    echo ${bots[$i]} " " ${bots[$j]}
    for k in `seq 1 $gamespermatch`
    do
      b1=${bots[$i]}
      b2=${bots[$j]}

      runname="$b1-$b2-$k"

      date=`date`
      info=`cat $runname.server.err | grep infoline`
      echo "$date $runname $info"

      winner=`echo $info | awk '{print $5}'`
      if [ $winner -ge 1 ]
      then
        wi=`expr $winner - 1`
        wins[$wi]=`expr ${wins[$wi]} + 1`
      fi

    done

    # now invert seating

    for k in `seq 1 $gamespermatch`
    do
      b2=${bots[$i]}
      b1=${bots[$j]}


      runname="$b1-$b2-$k"
      
      #echo "$runname"
      
      date=`date`
      info=`cat $runname.server.err | grep infoline`
      echo "$date $runname $info"
      
      winner=`echo $info | awk '{print $5}'`
      if [ $winner -ge 0 ]
      then
        wi=`expr 2 - $winner`
        wins[$wi]=`expr ${wins[$wi]} + 1`
      fi

    done

    
  done
done

for i in `seq 0 $lastindex`
do
  echo "${bots[$i]} wins: ${wins[$i]}"
done


