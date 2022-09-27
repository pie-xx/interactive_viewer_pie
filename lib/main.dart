import 'package:flutter/material.dart';
import 'dart:io';
import 'package:file_picker/file_picker.dart';
import 'interactive_image_viewer.dart';

void main() {
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

  @override
  void initState()  {
    // TODO: implement initState
    super.initState();

    iviewer_u = InteractiveImageViewer();

    iviewer_u.setOnDoubleTap((){
      print(iviewer_u.getTapPos());
      print(iviewer_u.getTapImgPos());
    });

  }
  void load() async {
    var res = await FilePicker.platform.pickFiles();
    if( res != null ){
      File file = File(res.files.single.path??"");
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
