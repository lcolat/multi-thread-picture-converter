# multi-thread-picture-converter

#### Launch project with docker
##### Windows
```bash
docker run --rm -v "%cd%":/usr/src/prog -it -w /usr/src/prog gcc:latest /bin/bash -c "gcc main.c -lpthread -o apply-effect && ./apply-effect "./in/" "./out/" <threadQuantity> <algorithm>
```
##### Linux
```bash
docker run --rm -v $(pwd):/usr/src/prog -it -w /usr/src/prog gcc:latest /bin/bash -c "gcc main.c -lpthread -o apply-effect && ./apply-effect "./in/" "./out/" <threadQuantity> <algorithm>
```

docker run --rm -v "%cd%":/usr/src/prog -it -w /usr/src/prog gcc:latest /bin/bash -c "gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all -o apply-effect && ./apply-effect"