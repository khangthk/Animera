//
//  cli.cpp
//  Animera
//
//  Created by Indiana Kernick on 23/11/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "cli.hpp"

#include "sprite.hpp"
#include "strings.hpp"
#include "file io.hpp"
#include "sprite file.hpp"
#include "application.hpp"
#include "docopt helpers.hpp"
#include <QtCore/qtextstream.h>

CLI::CLI(int &argc, char **argv)
  : argc{argc}, argv{argv} {}

namespace {

// docopt has terrible error messages for the | operator

const char docopt_usage[] =
R"(Usage:
    Animera [--help --long-help]
    Animera new <width> <height> [<format>]
    Animera open <file>
    Animera info [--json] <file>
    Animera export [--name=<pattern> --directory=<path>]
                   [--layer-stride=<int> --layer-offset=<int>]
                   [--frame-stride=<int> --frame-offset=<int>]
                   [--layer=<range> --frame=<range>]
                   [--no-composite --format=<format> --visibility=<mode>]
                   [--scale-x=<int> --scale-y=<int> --scale=<int>]
                   [--angle=<int>] <file>)";

const char usage[] =
R"(Usage:
    Animera [--help | --long-help]
    Animera new <width> <height> [<format>]
    Animera open <file>
    Animera info [--json] <file>
    Animera export [--name=<pattern> --directory=<path>]
                   [--layer-stride=<int> --layer-offset=<int>]
                   [--frame-stride=<int> --frame-offset=<int>]
                   [--layer=<range> --frame=<range>]
                   [--no-composite --format=<format> --visibility=<mode>]
                   [[--scale-x=<int> --scale-y=<int>] | --scale=<int>]
                   [--angle=<int>] <file>)";

const char short_options[] =
R"(Options:
    -h, --help                Display this help message.
    --long-help               Display a detailed help message.
    <width>                   Width of the sprite to create.
    <height>                  Height of the sprite to create.
    <format>                  Format of the sprite to create.
    -j, --json                Output info as JSON.
    -n, --name <pattern>      Name pattern for the sprite.
    -d, --directory <path>    Directory to write files to.
    --layer-stride <int>      Stride multiplied by layer number.
    --layer-offset <int>      Offset added to layer number.
    --frame-stride <int>      Stride multiplied by frame number.
    --frame-offset <int>      Offset added to frame number.
    -l, --layer <range>       Range of layers to export.
    -f, --frame <range>       Range of frames to export.
    -c, --no-composite        Don't composite layers.
    -F, --format <format>     Format of the output files.
    --visibility <mode>       Mode for handling layer visibility.
    --scale-x <int>           Horizontal scale factor applied to output images.
    --scale-y <int>           Vertical scale factor applied to output images.
    --scale <int>             Scale factor applied to output images.
    -a, --angle <int>         Angle of rotation applied to output images.)";

const char long_options[] =
R"(Options:
    -h, --help
        Display a brief help message.
    
    --long-help
        Display this help message.
    
    <width>
        Width (in pixels) of the sprite to create.
    
    <height>
        Height (in pixels) of the sprite to create.
    
    <format>
        Format of the sprite to create. Valid formats are:
            rgba   (8-bit RGBA)
            index  (8-bit Indexed)
            gray   (8-bit Grayscale with alpha)
        This is "rgba" by default.
    
    -j, --json
        By default, sprite info is outputted in a pleasant-for-humans format.
        When this option is present, sprite info is outputted as JSON.
    
    -n, --name <pattern>
        Name pattern for the sprite. By default this is "sprite_%000F".
        This pattern may contain format sequences.
        The available format sequences are:
            %F  frame number
            %L  layer number
        Zeros may appear after the % character to pad the number with zeros.
        Examples assuming that "e.animera" contains 2 layers and 2 frames:
            Animera export -n "a_%F" e.animera        a_0 a_1
            Animera export -n "b_%000F" e.animera     b_000 b_001
            Animera export -c -n "c_%L_%F" e.animera  c_0_0 c_0_1 c_1_0 c_1_1
    
    -d, --directory <path>
        Directory to write files to. By default this is ".". The output file
        paths are generated by concatenating this and the above options:
            <path> "/" <pattern> ".png"
        Paths ending with a / are accepted.
        
    --layer-stride <integer>
        This is similar to the --frame-stride option but for layers.
        
    --layer-offset <integer>
        This is similar to the --frame-offset option but for layers.
    
    --frame-stride <integer>
        A stride that is multiplied by the frame number before evaluating the
        name pattern. The stride cannot be 0 but can be negative.
        Examples assuming that "e.animera" contains 2 frames:
            Animera export -n "a_%F" --frame-stride 2 e.animera   a_0 a_2
            Animera export -n "b_%F" --frame-stride -1 e.animera  b_0 b_-1
        This option is useful when combined with the --frame-offset option.
    
    --frame-offset <integer>
        An offset that is added to the frame number before evaluating the name
        pattern. Examples assuming that "e.animera" contains 2 frames:
            Animera export -n "a_%F" --frame-offset 2 e.animera   a_2 a_3
            Animera export -n "b_%F" --frame-offset -1 e.animera  b_-1 b_0
        The offset is applied after the stride is applied.
    
    -l, --layer <range>
        This is similar to the --frame option but for layers.
    
    -f, --frame <range>
        The range of frames to export. By default, all frames are exported. This
        may be a single frame such as 0 or an inclusive range such as 1..3.
        Either or both sides of the range may be omitted.
        Examples assuming a sprite with 4 frames:
            1..2            frames 1 and 2
            ..2   or  0..2  frames 0, 1 and 2
            1..   or  1..3  frames 1, 2 and 3
            ..    or  0..3  frames 0, 1, 2 and 3
            1     or  1..1  frame 1
    
    -c, --no-composite
        By default, the cells that make up a frame are composited so that each
        frame is exported as a single image. If this option is present, layers
        are not composited and the cells that make up a frame are exported
        individually. The presence or absence of this option can affect the
        output formats available.
    
    -F, --format <format>
        The resulting output files are always PNGs. This option corresponds to
        the color type of the PNGs. The output formats available depend on the
        format of the input sprite. Given the input format, these are the valid
        output formats (the first in each list is the default):
            RGBA
                rgba        (8-bit RGBA)
            Indexed (with --no-composite)
                index       (8-bit Indexed)
                gray        (8-bit Grayscale)
                monochrome  (1-bit Grayscale)
            Indexed (without --no-composite)
                rgba        (8-bit RGBA)
            Grayscale
                gray-alpha  (8-bit Grayscale with alpha)
                gray        (8-bit Grayscale)
                monochrome  (1-bit Grayscale)
    
    --visibility <mode>
        The visibility of a layer in the range (defined by the --layer option)
        affects whether it will be included in the export. This option defines
        how the visibility affects inclusion. There are three possible values:
            visible  Visible layers are included
            hidden   Hidden layers are included
            all      All layers are included
        By default, this is in "visible" mode.
    
    --scale-x <integer>
        The horizontal scale factor applied to each output image. The scale
        factor cannot be 0 but can be negative, in this case, the image is
        flipped horizontally. If this option is present, the --scale option
        cannot be.
    
    --scale-y <integer>
        The vertical scale factor applied to each output image. The scale factor
        cannot be 0 but can be negative, in this case, the image is flipped
        vertically. If this option is present, the --scale option cannot be.
    
    -s, --scale <integer>
        The scale factor applied to each output image. This option is equivalent
        to using the --scale-x and --scale-y options with the same scale factor.
        If this option is present, neither --scale-x nor --scale-y can be.
    
    -a, --angle <integer>
        The angle of rotation applied to each output image. The image can only
        be rotated in 90 degree increments. This means that --angle 1 will
        rotate clockwise by 90 degrees. The angle doesn't need to be within
        [0, 4] so --angle -3, --angle 1 and --angle 5 are all equivalent.
        The rotation is applied after the scale is applied.)";

Error checkMutuallyExclusive(const std::map<std::string, docopt::value> &flags) {
  if (flags.at("--help").asBool() && flags.at("--long-help").asBool()) {
    return "--help must be mutually exclusive with --long-help";
  }
  const bool scaleXorY = flags.at("--scale-x") || flags.at("--scale-y");
  if (scaleXorY && flags.at("--scale")) {
    return "--scale-x and --scale-y must be mutually exclusive with --scale";
  }
  return {};
}

QTextStream &console() {
  static QTextStream stream{stdout};
  return stream;
}

}

int CLI::exec() {
  std::map<std::string, docopt::value> flags;
  if (Error err = parseArgs(flags); err) {
    console() << "Command line error\n" << err.msg() << '\n';
    console() << usage << '\n';
    return 1;
  }
  
  if (flags.at("new").asBool()) {
    return execNew(flags);
  } else if (flags.at("open").asBool()) {
    return execOpen(flags);
  } else if (flags.at("info").asBool()) {
    return execInfo(flags);
  } else if (flags.at("export").asBool()) {
    return execExport(flags);
  } else {
    return execDefault(flags);
  }
}

Error CLI::parseArgs(std::map<std::string, docopt::value> &flags) const {
  char **first = argv + 1;
  if (argc == 2 && std::strncmp(*first, "-psn_", 5) == 0) {
    ++first;
  }
  std::string doc = docopt_usage;
  doc += "\n\n";
  doc += short_options;
  try {
    flags = docopt::docopt_parse(doc, {first, argv + argc}, false, false);
  } catch (docopt::DocoptArgumentError &e) {
    return e.what();
  }
  return checkMutuallyExclusive(flags);
}

int CLI::execDefault(const std::map<std::string, docopt::value> &flags) const {
  if (flags.at("--help").asBool()) {
    console() << usage << "\n\n" << short_options << '\n';
    return 0;
  }
  if (flags.at("--long-help").asBool()) {
    console() << usage << "\n\n" << long_options << '\n';
    return 0;
  }
  Application app{argc, argv};
  app.waitForOpenEvent();
  return app.exec();
}

namespace {

const char *formatNames[] = {
  "rgba",
  "index",
  "gray"
};

Error setFormatSize(Format &format, QSize &size, const std::map<std::string, docopt::value> &flags) {
  TRY(setInt(size.rwidth(), flags.at("<width>"), "width", init_size_range));
  TRY(setInt(size.rheight(), flags.at("<height>"), "height", init_size_range));
  if (const docopt::value &value = flags.at("<format>"); value) {
    if (!setEnum(format, value.asString(), formatNames)) {
      return "Invalid format" + validListStr("formats", formatNames);
    }
  } else {
    format = Format::rgba;
  }
  return {};
}

}

int CLI::execNew(const std::map<std::string, docopt::value> &flags) const {
  Format format;
  QSize size;
  if (Error err = setFormatSize(format, size, flags); err) {
    console() << "Configuration error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  Application app{argc, argv};
  app.newFile(format, size);
  return app.exec();
}

int CLI::execOpen(const std::map<std::string, docopt::value> &flags) const {
  Application app{argc, argv};
  app.openFile(toLatinString(flags.at("<file>").asString()));
  return app.exec();
}

namespace {

Error readInfo(const QString &path, SpriteInfo &info) {
  FileReader reader;
  TRY(reader.open(path));
  TRY(readSignature(reader.dev()));
  TRY(readAHDR(reader.dev(), info));
  TRY(reader.flush());
  return Error{};
}

QString formatToString(const Format format) {
  switch (format) {
    case Format::rgba:
      return "RGBA";
    case Format::index:
      return "Indexed";
    case Format::gray:
      return "Grayscale";
    default:
      return "";
  }
}

}

int CLI::execInfo(const std::map<std::string, docopt::value> &flags) const {
  QCoreApplication app{argc, argv};
  SpriteInfo info;
  
  if (Error err = readInfo(toLatinString(flags.at("<file>").asString()), info); err) {
    console() << "File open error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  
  // TODO: Maybe an option to output LHDR
  
  if (flags.at("--json").asBool()) {
    console() << "{\n";
    console() << "  \"width\": " << info.width << ",\n";
    console() << "  \"height\": " << info.height << ",\n";
    console() << "  \"format\": \"" << formatToString(info.format) << "\",\n";
    console() << "  \"layers\": " << static_cast<int>(info.layers) << ",\n";
    console() << "  \"frames\": " << static_cast<int>(info.frames) << ",\n";
    console() << "  \"delay\": " << info.delay << '\n';
    console() << "}\n";
  } else {
    console() << "Size:   {" << info.width << ", " << info.height << "}\n";
    console() << "Format: " << formatToString(info.format) << '\n';
    console() << "Layers: " << static_cast<int>(info.layers) << '\n';
    console() << "Frames: " << static_cast<int>(info.frames) << '\n';
    console() << "Delay:  " << info.delay << " ms\n";
  }
  
  return 0;
}

int CLI::execExport(const std::map<std::string, docopt::value> &flags) const {
  QCoreApplication app{argc, argv};
  Sprite sprite;
  if (Error err = sprite.openFile(toLatinString(flags.at("<file>").asString())); err) {
    console() << "File open error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  ExportOptions options;
  const ExportSpriteInfo info = getSpriteInfo(sprite);
  initDefaultOptions(options, info);
  if (Error err = readExportOptions(options, info, flags); err) {
    console() << "Configuration error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  if (Error err = sprite.exportSprite(options); err) {
    console() << "Export error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  return 0;
}
