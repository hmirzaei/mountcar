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

echo '<p style="margin:0;padding:0;font-weight:bold;color:blue;">Building the project...</p><pre>' > progress.txt
cat progress.txt | nc cps.ics.uci.edu 8080
cat includes.h rlalg.c > temp
mv temp rlalg.c

make >> progress.txt 2>&1
ret_code=$?
echo '</pre>' >> progress.txt
cat progress.txt | nc cps.ics.uci.edu 8080
if [ $ret_code -eq 0 ]; then
    echo '<p style="margin:0;padding:0;font-weight:bold;color:green;">Build finished.</p>' >> progress.txt
    echo '<p style="margin:0;padding:0;font-weight:bold;color:blue;">Running...</p><pre>' >> progress.txt
    cat progress.txt | nc cps.ics.uci.edu 8080
    sudo ./mountcar -80 60000 30 >> progress.txt 2>&1
    ret_code=$?
    echo '</pre>' >> progress.txt
    cat progress.txt | nc cps.ics.uci.edu 8080
    if [ $ret_code -eq 0 ]; then
	echo '<p style="margin:0;padding:0;font-weight:bold;color:green;">Finished running.</p>' >> progress.txt
	cat progress.txt | nc cps.ics.uci.edu 8080
    else
	echo '<p style="margin:0;padding:0;font-weight:bold;color:red;">RUNTIME ERROR!</p>' >> progress.txt
	cat progress.txt | nc cps.ics.uci.edu 8080
    fi
else
    echo '<p style="margin:0;padding:0;font-weight:bold;color:red;">BUILD ERROR!</p>' >> progress.txt
    cat progress.txt | nc cps.ics.uci.edu 8080
fi
echo '<p style="margin:0;padding:0;font-weight:bold;color:green;">done.</p>' >> progress.txt
cat progress.txt | nc cps.ics.uci.edu 8080

sh pwm.sh 0 0
rmdir "$lockdir"
