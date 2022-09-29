import 'dart:io';
import 'dart:ffi';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';


class OpenCVFFi {
  
  late DynamicLibrary  dylib ;
  late String outjpg;

  String filter_name = "NON";
  static List<String> filter_list = [];

  late Pointer<Uint32> s;
  late Pointer<Uint32> w;
  late Pointer<Uint32> h;
  late Pointer<Uint32> cr;
  late Pointer<Uint32> cg;
  late Pointer<Uint32> cb;

  int imgwidth=0;
  int imgheight=0;

  OpenCVFFi(String _outjpg){

    s = malloc.allocate(1);
    w = malloc.allocate(1);
    h = malloc.allocate(1);

    cr = malloc.allocate(1);
    cg = malloc.allocate(1);
    cb = malloc.allocate(1);

    outjpg = _outjpg;

    dylib = Platform.isAndroid
      ? DynamicLibrary.open("libOpenCV_ffi.so")
      : DynamicLibrary.open("ImgProcDll.dll");

    final getfnum = dylib.lookupFunction<
        Void Function(Pointer<Uint32>),
        void Function(Pointer<Uint32>)
        >("getlen");
    getfnum(s);
    print("getlen ${s[0]}");

    int tf = s[0];

    String fnamebuff = "0123456789abcdef";
    Pointer<Uint8> buff = fnamebuff.toNativeUtf8().cast<Uint8>();;

    final getname = dylib.lookupFunction<
        Void Function(Pointer<Uint32>, Pointer<Uint8>, Pointer<Uint32>),
        void Function(Pointer<Uint32>, Pointer<Uint8>, Pointer<Uint32>)
        >("getname");


    Uint8List blist = buff.asTypedList(128);

    filter_list.clear();
    for( int n=0; n < tf; ++n){
      s[0] = n; w[0]=fnamebuff.length;
      getname(s,buff,w); 
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

  static List<String> getFilterNameList(){
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

  void filter( String curfile ) {
    final outPath = outjpg.toNativeUtf8().cast<Uint8>();
    final inPath = curfile.toNativeUtf8().cast<Uint8>();
    try {

      filter_name ="BW";
      s[0] = 20;

      final gaussian = dylib.lookupFunction<
        Void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>),
        void Function(Pointer<Uint8>, Pointer<Uint8>, Pointer<Uint32>, Pointer<Uint32>, Pointer<Uint32>)
        >(filter_name);

      gaussian(inPath, outPath, s, w, h);

      imgwidth = w[0];
      imgheight= h[0];

    }catch(e){
      print(e.toString());
    }



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
