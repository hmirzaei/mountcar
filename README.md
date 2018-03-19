Deployment
----------

### On Web server:

- Run opebstream/main.py on an screen:
```bash
cd opebstream
screen
python main.py
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

- Just as a quick test, run mountcar on pi
```bash
cd rp
make
sudo ./mountcar -80 100000 30
```


