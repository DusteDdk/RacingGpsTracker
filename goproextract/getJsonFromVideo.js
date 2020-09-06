const goproTelemetry = require('gopro-telemetry');
const gpmfExtract = require('gpmf-extract');
const fs = require('fs');

const fn = process.argv[2];
if(!fn || fn === '--help' || fn === '-h') {
    console.log('Take name of a video file and extract JSON GPS stream');
    console.log('required: name of video file to extract');
    process.exit(1);
}
gpmfExtract(bufferAppender(fn, 10 * 1024 * 1024)).then( res => {
    const options = {
        stream: ['GPS5'],
    };

    goproTelemetry(res, options, (data)=>{
        console.log(JSON.stringify(data,null,4));
    });
});


function bufferAppender(path, chunkSize) {
  return function (mp4boxFile) {
    var stream = fs.createReadStream(path, { highWaterMark: chunkSize });
    var bytesRead = 0;
    stream.on('end', () => {
      mp4boxFile.flush();
    });
    stream.on('data', chunk => {
      var arrayBuffer = new Uint8Array(chunk).buffer;
      arrayBuffer.fileStart = bytesRead;
      mp4boxFile.appendBuffer(arrayBuffer);
      bytesRead += chunk.length;
    });
    stream.resume();
  };
}
