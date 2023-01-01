import 'dart:io';

import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:path_provider/path_provider.dart';

import 'interactive_image_viewer.dart';
import 'opencv_ffi.dart';

late String injpg;
late String outjpg;

late String dispjpg;
late String chart1jpg;
late String chart2jpg;

const double _panel_height = 160;
const double _panel_width = 240;
const double _histgph_width = 512;

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  Directory _tdir = await getApplicationDocumentsDirectory();
  outjpg = _tdir.path+"/ivp_output0.jpg";

  dispjpg = _tdir.path+"/ivp_disp.jpg";
  chart1jpg = _tdir.path+"/ivp_chart1.jpg";
  chart2jpg = _tdir.path+"/ivp_chart2.jpg";

  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData( primarySwatch: Colors.blue, ),
      home: const MyHomePage(title: '12'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, required this.title}) : super(key: key);
  final String title;
  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  late Image img ;
  late InteractiveImageViewer ivmain;
  late InteractiveImageViewer ivsub;
  late InteractiveImageViewer ivchart1;
  late InteractiveImageViewer ivchart2;
  late OpenCVFFi _opencvffi;

  int blim = 0;
  int wlim = 256;

  Offset? area_pos;
  int area_width=0;
  int area_height=0;

  List<DropdownMenuItem<int>> _items = [];
  int _selectItem = 0;

  @override
  void initState()  {
    // TODO: implement initState
    super.initState();

    _opencvffi = OpenCVFFi(outjpg);
    ivmain = InteractiveImageViewer();
    ivsub = InteractiveImageViewer();
    ivchart1 = InteractiveImageViewer();
    ivchart2 = InteractiveImageViewer();

    const TextStyle dropstyle = TextStyle(fontSize: 20.0, 
        color: Colors.white, backgroundColor: Colors.blue, );

    int v=0;
    for( String fname in _opencvffi.getFilterNameList() ){
      _items.add( DropdownMenuItem(
          child: Text(fname, style: dropstyle,),
          value: v,
      ));
      ++v;
    }
    ivchart1.setOnDoubleTap(() async {
      Offset _pos = ivchart1.getTapImgPos();
      if(_pos.dy > _panel_height /2 ){
        blim = ((_pos.dx / _histgph_width) * 256).toInt();
        if(blim < 0){
          blim = 0;
        }
        if(blim > 256){
          blim = 256;
        }
      }else{
        wlim = ((_pos.dx / _histgph_width) * 256).toInt();
        if(wlim < 0){
          wlim = 0;
        }
        if(wlim > 256){
          wlim = 256;
        }
      }
      setState(() {
        
      });
      _opencvffi.bw2(injpg, outjpg, blim, wlim);
      File file = File(outjpg);
      final imageForUint8 = await file.readAsBytes();
      Image _img = await Image.memory(imageForUint8);

      ivmain.loadimage(_img);
    });

    ivmain.setOnDoubleTap(() async {

      print(ivmain.getTapPos());
      print(ivmain.getTapImgPos());
      Offset _pos = ivmain.getTapImgPos();
      /*
      List cols = _opencvffi.getPosColor(outjpg, _pos.dx.toInt(), _pos.dy.toInt());
      print(cols);

      _opencvffi.mkchart(outjpg, chart1jpg, chart2jpg, _pos.dx.toInt(), _pos.dy.toInt());
      File sfile = File(chart1jpg);
      final imageForUint8s = await sfile.readAsBytes();
      ivchart1.loadimage(await Image.memory(imageForUint8s));

      File sfile2 = File(chart2jpg);
      final imageForUint8s2 = await sfile2.readAsBytes();
      ivchart2.loadimage(await Image.memory(imageForUint8s2));

      //_opencvffi.dispTap(outjpg, dispjpg,  _pos.dx.toInt(), _pos.dy.toInt());
      //int dir = _opencvffi.isHorV(injpg, dispjpg,  _pos.dx.toInt(), _pos.dy.toInt());
      //_opencvffi.markChar(injpg, outjpg, _pos.dx.toInt(), _pos.dy.toInt());

      File sfile3 = File(outjpg);
      final imageForUint8s3 = await sfile3.readAsBytes();
      ivmain.loadimage(await Image.memory(imageForUint8s3));
      */
      _opencvffi.lochistgram( injpg, chart1jpg, _histgph_width.toInt(), _panel_height.toInt(), _pos.dx.toInt(), _pos.dy.toInt() ) ;
      File file = File(chart1jpg);
      final imageForUint8 = await file.readAsBytes();
      Image _img = await Image.memory(imageForUint8);
      ivchart1.loadimage(_img);
      /**/
    });

    ivmain.setOnLongPressDown((){
      area_pos = ivmain.getTapImgPos();
    });
    ivmain.setOnLongPressEnd(()async{
      Offset pos = ivmain.getTapImgPos();
      int sx = area_pos!.dx.toInt();
      int sy = area_pos!.dy.toInt();
      int width;
      int height;
      if( pos.dx < sx ){
        width = (sx - pos.dx).toInt();
        sx = pos.dx.toInt();
      }else{
        width = (pos.dx-sx).toInt();
      }
      if( pos.dy < sy ){
        height = (sy - pos.dy).toInt();
        sy = pos.dy.toInt();
      }else{
        height = (pos.dy-sy).toInt();
      }

      _opencvffi.rectangle(injpg, outjpg, sx, sy, width, height);
      File fileM = File(outjpg);
      final imageForUint8 = await fileM.readAsBytes();
      Image _imgM = await Image.memory(imageForUint8);
      ivmain.loadimage(_imgM);
    });
  }

  void trimtextimg() async {
      _opencvffi.trimspace(outjpg, chart1jpg, chart2jpg);

      File sfile = File(chart1jpg);
      final imageForUint8s = await sfile.readAsBytes();
      ivchart1.loadimage(await Image.memory(imageForUint8s));

      File sfile2 = File(chart2jpg);
      final imageForUint8s2 = await sfile2.readAsBytes();
      ivchart2.loadimage(await Image.memory(imageForUint8s2));
  }

  void histgram() async{
    _opencvffi.histgram(outjpg, chart1jpg, _histgph_width.toInt(), _panel_height.toInt());
    File file = File(chart1jpg);
    final imageForUint8 = await file.readAsBytes();
    Image _img = await Image.memory(imageForUint8);
    ivchart1.loadimage(_img);
  }

  void load() async {
    var res = await FilePicker.platform.pickFiles();
    if( res != null ){
      injpg = res.files.single.path??"";
      //_opencvffi.markCharMap(injpg, outjpg);
      filter_image();



      File file = File(injpg);
      final imageForUint8 = await file.readAsBytes();
      Image _img = await Image.memory(imageForUint8);

      ivsub.loadimage(_img);

    }
  }
  void filter_image() async {
    //if(_selectItem==0){
    //  File fileM = File(injpg);
    //  Image _imgM = await Image.file(fileM);
    //  ivmain.loadimage(_imgM);
    //}else{
      _opencvffi.filter(_selectItem, injpg, outjpg);
      File fileM = File(outjpg);
      //Image _imgM = await Image.file(fileM);
      
      final imageForUint8 = await fileM.readAsBytes();
      Image _imgM = await Image.memory(imageForUint8);
      ivmain.loadimage(_imgM);
    //}
  }

  @override
  Widget build(BuildContext context) {
    var scwidth = MediaQuery.of(context).size.width;
    var scheight = MediaQuery.of(context).size.height - kToolbarHeight /*-kBottomNavigationBarHeight-2*/ -MediaQuery.of(context).padding.top ;

    File file = File(widget.title);
    //img = Image.file(file);

    return Scaffold(
      appBar: AppBar(
        title: Text(file.path.substring(file.parent.path.length+1)),
        actions: [
          IconButton(
            icon: Icon(Icons.crop, color: Colors.white,),
            tooltip: 'trim',
            onPressed: () {
              trimtextimg(  );
            },
          ),
          IconButton(
            icon: Icon(Icons.bar_chart, color: Colors.white,),
            tooltip: 'histgram',
            onPressed: () {
              histgram(  );
            },
          ),
          IconButton(
            icon: Icon(Icons.read_more, color: Colors.white,),
            tooltip: 'read',
            onPressed: () {
              setState(() {
              load();
              histgram();
              });
            },
          ),
          DropdownButton(
                    items: _items,
                    value: _selectItem,
                    dropdownColor: Colors.blue,
                    onChanged: (value) {
                      _selectItem = value as int;
                        filter_image();
                        setState(() {    
                        });
                    }
          ),
        ],

      ),
      body: Column( 
              children: [
              Row( children:[
                SizedBox(
                  width: scwidth*2/3,
                  height: _panel_height,
                  child: ivchart1),
                Row(
                  children:[
                    Column(
                      children:  [
                        Text("blim $blim"),
                        Text("wlim $wlim"),
                      ],
                    ),
                    SizedBox(
                      width: scwidth/5,
                      height: _panel_height,
                      child: ivsub),
                  ]
                ),
              ]),
              Row(children: [
                SizedBox(
                width: scwidth*2/3,
                height: scheight-_panel_height,
                child: ivmain), 
                SizedBox(
                width: scwidth/3,
                height: scheight-_panel_height,
                child: ivchart2), 
              ],)
            ]),
      /*
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          load();
        },
        tooltip: 'load',
        child: const Icon(Icons.read_more),
      ),
      */ // This trailing comma makes auto-formatting nicer for build methods.
    );
  }
}
