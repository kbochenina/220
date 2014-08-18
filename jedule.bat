set currentDir=%CD%
D:
cd \
cd "D:\ITMO\Degree\Programs\WFSched\Output"
If Not Exist Images md Images
java -jar jedule.jar JeduleStarter -p simgrid -d 1024x768 -o Images\Simple_0.png -gt png -cm ..\cmap.xml -f Simple_0.jed
java -jar jedule.jar JeduleStarter -p simgrid -d 1024x768 -o Images\Ordered_0.png -gt png -cm ..\cmap.xml -f Ordered_0.jed
java -jar jedule.jar JeduleStarter -p simgrid -d 1024x768 -o Images\Clustered_0.png -gt png -cm ..\cmap.xml -f Clustered_0.jed
cd /d %currentDir%