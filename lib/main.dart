import 'dart:io';

import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:path_provider/path_provider.dart';

import 'interactive_image_viewer.dart';
import 'opencv_ffi.dart';

late String outjpg;

void main() async {

  Directory _tdir = await getApplicationDocumentsDirectory();
  outjpg = _tdir.path+"/ivp_output.jpg";

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
  var iviewer_u;
  late OpenCVFFi _opencvffi;

  @override
  void initState()  {
    // TODO: implement initState
    super.initState();

    _opencvffi = OpenCVFFi(outjpg);
    iviewer_u = InteractiveImageViewer();

    iviewer_u.setOnDoubleTap((){
      print(iviewer_u.getTapPos());
      print(iviewer_u.getTapImgPos());
    });
  }

  void load() async {
    var res = await FilePicker.platform.pickFiles();
    if( res != null ){
      _opencvffi.filter(res.files.single.path??"");
      File file = File(outjpg);
      final imageForUint8 = await file.readAsBytes();
      iviewer_u.loadimage(await Image.memory(imageForUint8));
    }
  }
  @override
  Widget build(BuildContext context) {
    File file = File(widget.title);
    img = Image.file(file);

    return Scaffold(
      appBar: AppBar(
        title: Text(file.path.substring(file.parent.path.length+1)),
      ),
      body: iviewer_u,
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          load();
        },
        tooltip: 'load',
        child: const Icon(Icons.read_more),
      ), // This trailing comma makes auto-formatting nicer for build methods.
    );
  }
}
