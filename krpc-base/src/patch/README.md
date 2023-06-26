# PATCH

## How to use
### Create patch
1. Copy files
```
$ bash ./copy_file.sh
```

2. Fix files
```
$ vim <filepath>.new
```

3. Create patchs
```
$ bash ./make_patch.sh
```
*attention*
You should do 1-3 steps before building docker.


### Apply patch
```
# You should run in Dockerfile.
$ bash ./apply_patch.sh
```
