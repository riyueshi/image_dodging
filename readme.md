##introduction   
this code implement a simple image dodging process. which is implement by hist matching on light channel of HLS color space. firstly, convert the RBG image to HLS image color space,then implement the hist matching on L(light) channel.
##command line   
the .exe file accept 4 strings as parameters:   
argv[1]  -the standard image name;   
argv[2]  -image type.;   
argv[3]  -source images folder name;   
argv[4]  -result images folder name;   
   
use example:   
image_dodging.exe "D:\\work_path\\standard_image.jpg" "jpg" "D:\\work_path" "D:\\work_path\\out"   
this code doesn't create path or folder, so make sure that the source images folder and the result images folder are already exit in you computer.
you can double click the run.bat file to run a simple example.   
##comments   
this code is tested on windows 7 + vs2013 + opencv 3.0 and a opencv_world300.dll is necessary, which included in the opencv 3.0 package. the .exe file is a x64 release version, make sure your "opencv_world300.dll" is the corresponding one.the opencv package is available [here](http://sourceforge.net/projects/opencvlibrary/files/opencv-win/3.0.0/opencv-3.0.0.exe/download).    
this code has enable openMP process which can accelerate the process, at the same time, a high CPU occupied will needed. if you don't want use the openMP process comments the line "#pragma omp parallel for". And if you want to enable openMP process in you project you have to turn the use openMP attribute on for you visual studio project.   
It would be great to give the code a star on my github project if you think this code help.  