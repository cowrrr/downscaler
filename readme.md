# compile:

```
meson setup build
cd build
ninja
ninja install
```

## usage:

`downscaler -i <image>`

`-i` || `-input`   # input image  
`-v` || `-verbose` # verbose  
`-t` || `-target`  # target size in MB  
`-ma`              # max attempts to find closest size  
`-r`               # how many % below target is acceptable  

## configuration

### linux:

the default config lives in `~/.config/downscaler/config.ini`  
fallback config lives in `/usr/local/share/downscaler/config.ini`  

### Mac OS

the default config lives in `~/Library/Application Support/downscaler/config.ini`  
fallback config lives in `/usr/local/share/downscaler/config.ini`  
