import 'package:flutter/material.dart';
import 'dart:io';
import 'package:file_picker/file_picker.dart';

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

  @override
  Widget build(BuildContext context) {
    File file = File(widget.title);
    img = Image.file(file);

    return Scaffold(
      appBar: AppBar(
        title: Text(file.path.substring(file.parent.path.length+1)),
      ),
      body: InteractiveViewer(
            boundaryMargin: const EdgeInsets.all(20.0),
            minScale: 0.1,
            maxScale: 64,
            child: Center(child: img),
          ),
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          FilePickerResult? result = await FilePicker.platform.pickFiles(type: FileType.image);
          if(result != null) {
            File file = File(result.files.single.path??"");
            Navigator.push(
              this.context,
              MaterialPageRoute(
                builder: (context) => MyHomePage(title: file.path)
              )
         );
        }
        },
        tooltip: 'load',
        child: const Icon(Icons.add),
      ), // This trailing comma makes auto-formatting nicer for build methods.
    );
  }
}
