//import 'dart:html';
import 'dart:io';
import 'dart:ffi';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';


class OpenCVFFi {
  
  late DynamicLibrary  dylib ;
  late String outjpg;

  late Function getlen;
  late Function getname;
  

  String filter_name = "NON";
  List<String> filter_list = [];

  late Pointer<Uint32> s;
  late Pointer<Uint32> w;
  late Pointer<Uint32> h;
  late Pointer<Uint32> cr;
  late Pointer<Uint32> cg;
  late Pointer<Uint32> cb;

  int imgwidth=0;
  int imgheight=0;

  OpenCVFFi(String _outjpg){

    s = malloc.allocate(1024);
    w = malloc.allocate(1024);
    h = malloc.allocate(1024);

    cr = malloc.allocate(1);
    cg = malloc.allocate(1);
    cb = malloc.allocate(1);

    outjpg = _outjpg;

    dylib = Platform.isAndroid
      ? DynamicLibrary.open("libOpenCV_ffi.so")
      : DynamicLibrary.open("ImgProcDll.dll");

    getlen = dylib.lookupFunction<
        Uint32 Function(),
        int Function()
        >("getlen");

    print("getlen ${getlen()}");

    int tf = getlen();

    String fnamebuff = "0123456789abcdef";
    Pointer<Uint8> buff = fnamebuff.toNativeUtf8().cast<Uint8>();

    getname = dylib.lookupFunction<
        Void Function(Uint32, Pointer<Uint8>, Pointer<Uint32>),
        void Function(int, Pointer<Uint8>, Pointer<Uint32>)
        >("getname");

    Uint8List blist = buff.asTypedList(128);

    filter_list.clear();
    for( int n=0; n < tf; ++n){
      s[0] = n; w[0]=fnamebuff.length;
      getname(n,buff,w); 
      String fname = String.fromCharCodes(blist,0,w[0]);
      print("flutter $n ${ fname }");
      filter_list.add(fname);
    }

  }

  int getWidth(){
    return imgwidth;
  }
  int getHeight(){
    return imgheight;
  }

  List<String> getFilterNameList(){
    return filter_list;
  }

  void setFilterName(String name){
    setFilterNo( filter_list.indexOf(name));
  }
  String getFilterName(){
    return filter_name;
  }

  void setFilterNo(int no){
    if( no < 0 || no >= filter_list.length ){
      no = 0;
    }
    filter_name = filter_list[no];
  }
  int getFilterNo(){
    return filter_list.indexOf(filter_name);
  }

  void filter( int filterno, String curfile, String outfile1 ) {
    String filtername = filter_list[filterno];
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath = outfile1.toNativeUtf8().cast<Uint8>();
    try {
      s[0] = 15;
      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >(filtername);
      gaussian(inPath, outPath, s, w );
      print("filter $filtername width ${w[0]}, height ${w[1]}");

    }catch(e){
      print(e.toString());
    }
  }
  void bw2( String curfile, String outfile1, int blim, int wlim ) {

    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath = outfile1.toNativeUtf8().cast<Uint8>();
    try {
      s[0] = blim;
      s[1] = wlim;
      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("BW2");
      gaussian(inPath, outPath, s, w );
      print("filter BW2 width ${w[0]}, height ${w[1]}");

    }catch(e){
      print(e.toString());
    }
  }

  void histgram( String curfile, String chart1jpg , int width, int height ) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final chartPath = chart1jpg.toNativeUtf8().cast<Uint8>();

    try {
      final makeHistgramList = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint32>)
        >("makeHistgramList");
      makeHistgramList( inPath, w );

      //for( int n=0; n<256; ++n){
      //  print("$n: ${w[n]}");
      //}
      final makeHistgramChart = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("makeHistgramChart");
      s[0]=0; s[1]=0; s[2]=width; s[3]=height;
      makeHistgramChart( chartPath, s, w );

    }catch(e){
      print(e.toString());
    }
  }

  void lochistgram( String curfile, String chart1jpg , int width, int height, int px, int py ) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final chartPath = chart1jpg.toNativeUtf8().cast<Uint8>();

    try {
      //for( int n=0; n<256; ++n){
      //  w[n]=0;
      //}
      final makeLocalHistgramList = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("makeLocalHistgramList");
      s[0]=px; s[1]=py;
      makeLocalHistgramList( inPath, s, w );

      //for( int n=0; n<256; ++n){
      //  print("$n: ${w[n]}");
      //}
      final makeHistgramChart = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("makeHistgramChart");
      s[0]=0; s[1]=0; s[2]=width; s[3]=height;
      makeHistgramChart( chartPath, s, w );

    }catch(e){
      print(e.toString());
    }
  }
  
  void rectangle(String infile, String outfile, int sx, int sy, int width, int height){    
    final inPath = infile.toNativeUtf8().cast<Uint8>();
    final outPath1 = outfile.toNativeUtf8().cast<Uint8>();
    s[0]=sx;
    s[1]=sy;
    s[2]=width;
    s[3]=height;
    s[4]=255;
    s[5]=0;
    s[6]=0;
    s[7]=3;
    try {
      final func = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("rectangle");

      func(inPath, outPath1, s, w);

    }catch(e){
      print(e.toString());
    }
  }

  int isHorV( String curfile, String outfile, int x, int y ) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath1 = outfile.toNativeUtf8().cast<Uint8>();
    try {

      s[0] = x;
      s[1] = y;

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("isHorV");

      gaussian(inPath, outPath1, s, w);

      return w[0];

    }catch(e){
      print(e.toString());
    }
    return -1;
  }

  void markChar( String curfile, String outfile1, int x, int y ) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath1 = outfile1.toNativeUtf8().cast<Uint8>();
    try {
      //s[0] = direction;
      s[0] = x;
      s[1] = y;

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("markChar");

      gaussian(inPath, outPath1, s, w);

    }catch(e){
      print(e.toString());
    }
  }
/*  
  void markCharMap( String curfile, String outfile1){
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath1 = outfile1.toNativeUtf8().cast<Uint8>();
    try {

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("markCharMap");

      gaussian(inPath, outPath1, s, w);

    }catch(e){
      print(e.toString());
    }
  }

  void bw2( String curfile, String outfile1){
    final inPath = curfile.toNativeUtf8();
    final outPath1 = outfile1.toNativeUtf8().cast<Uint8>();
    try {
      s[0] = 15;

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Utf8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Utf8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>)
        >("BW2");

      gaussian(inPath, outPath1, s, w, h);

    }catch(e){
      print(e.toString());
    }
  }
*/
  //void filter( String filtername, String curfile, String outfile1 ){

  //}

  void mkchart( String curfile, String outfile1, String outfile2, int x, int y ) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath1 = outfile1.toNativeUtf8().cast<Uint8>();
    final outPath2 = outfile2.toNativeUtf8().cast<Uint8>();
    try {

      s[0] = x;
      s[1] = y;

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("XYGrayChart");

      gaussian(inPath, outPath1, outPath2, s, w);

    }catch(e){
      print(e.toString());
    }
  }

  void trimspace( String curfile, String outfile1, String outfile2) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath1 = outfile1.toNativeUtf8().cast<Uint8>();
    final outPath2 = outfile2.toNativeUtf8().cast<Uint8>();
    try {

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("LongSpaceChart");

      gaussian(inPath, outPath1, outPath2, s, w);

    }catch(e){
      print(e.toString());
    }
  }

  void dispTap( String curfile, String outfile1, int x, int y ) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath1 = outfile1.toNativeUtf8().cast<Uint8>();
    try {

      s[0] = x;
      s[1] = y;

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("dispTap");

      gaussian(inPath, outPath1, s, w);

    }catch(e){
      print(e.toString());
    }
  }

  List<int> getPosColor( String curfile, int x, int y ) {
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    final outPath = outjpg.toNativeUtf8().cast<Uint8>();
    List<int> poscolor = [-1,-1,-1];
    try {

      filter_name ="getPosColor";
      s[0] = x;
      s[1] = y;

      final getPosColor = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>)
        >("getPosColor");

      getPosColor(inPath, outPath, s, w);

      poscolor[0] = w[0];
      poscolor[1] = w[1];
      poscolor[2] = w[2];


    }catch(e){
      print(e.toString());
    }
    return poscolor;
  }

  void circle( String curfile, String out, int radius, int px, int py, int r, int g, int b ){
    final outPath = out.toNativeUtf8().cast<Uint8>();
    final inPath = curfile.toNativeUtf8().cast<Uint8>();

    final ocv_circle = dylib.lookupFunction<
      Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>),
      void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>)
      >("circle");

    s[0] = radius;
    w[0] = px;
    h[0] = py;
    cr[0] = r;
    cg[0] = g;
    cb[0] = b;

    ocv_circle(inPath, outPath, s, w, h, cb, cg, cr);
  }
}
