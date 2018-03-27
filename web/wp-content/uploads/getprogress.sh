while(true); do 
    nc -l 8080 > temp
    cp temp progress
done
