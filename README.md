Deployment
----------

### On Web server:

- Run opebstream/main.py in a screen:
```bash
cd opebstream
screen
python main.py
```

- Run mountcar/web/wp-content/uploads/getprogress.sh in a screen:
```bash
cp web/wp-content/uploads/getprogress.sh  /var/www/html/wp-content/uploads/.
cd /var/www/html/wp-content/uploads
screen
bash getprogress.sh
```

- Copy php scripts and additional files to html folder:
```bash
cp -r web/* /var/www/html/.
```

### On Raspberry Pi:

- copy runstart.sh and runstop.sh to home:
```bash
cp runstart.sh ~
cp runstop.sh ~
make
```
- build the project:
```bash
cd mountcar/rp
make
```

- change Makefile to build only the dynamic library:
```
all:
	gcc -Wall -fPIC rlalg.c -shared -Wl,-soname,librlalg.so -o librlalg.so 
```

