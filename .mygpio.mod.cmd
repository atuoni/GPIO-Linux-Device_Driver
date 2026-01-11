savedcmd_/home/atuoni/projeto2/mygpio.mod := printf '%s\n'   mygpio.o | awk '!x[$$0]++ { print("/home/atuoni/projeto2/"$$0) }' > /home/atuoni/projeto2/mygpio.mod
