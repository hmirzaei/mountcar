lockdir=/tmp/start.lock
if ! mkdir "$lockdir"; then
    echo "" > progress.txt
    cat progress.txt | nc cps.ics.uci.edu 8080
    PID=`ps a -o pid,cmd | grep '[0-9][0-9]* bash start.sh' | grep -v $$ | awk '{print $1}'`
    sudo kill -9 $PID
    wait $PID
    PID=`ps a -o pid,cmd | grep '[0-9][0-9]* gcc' | awk '{print $1}'`
    sudo kill -9 $PID
    wait $PID
    PID=`ps a -o pid,cmd | grep '[0-9][0-9]* make' | awk '{print $1}'`
    sudo kill -9 $PID
    wait $PID
fi
PID=`ps ax -o pid,cmd | grep '[0-9][0-9]* sudo ./mountcar' | awk '{print $1}'`
sudo kill -9 $PID
wait $PID
PID=`ps ax -o pid,cmd | grep '[0-9][0-9]* ./mountcar' | awk '{print $1}'`
sudo kill -9 $PID
wait $PID
echo '<p style="margin:0;padding:0;font-weight:bold;color:blue;">Stopped!</p><pre>' > progress.txt
cat progress.txt | nc cps.ics.uci.edu 8080
echo '<p style="margin:0;padding:0;font-weight:bold;color:green;">done.</p>' >> progress.txt
cat progress.txt | nc cps.ics.uci.edu 8080

sh pwm.sh 0 0
rmdir "$lockdir"
