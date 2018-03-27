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

- Generate the html document and copy it to html folder:
```bash
# after installing Bokeh
cd bokeh
# overwrite bokeh html template
sudo cp file.html /usr/lib/python2.7/site-packages/bokeh/core/_templates/file.html
python main.py > opeb.html
sudo cp opeb.html /var/www/html/.
```

- Copy php scripts and additional files to html folder:
```bash
cp -r web/* /var/www/html/.
```

### On Raspberry Pi:

- copy runstart.sh to home:
```bash
cp runstart.sh ~
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

