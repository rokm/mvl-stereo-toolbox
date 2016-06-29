function matrix = read_binary_matrix_file (filename)
    % matrix = READ_BINARY_MATRIX_FILE (filename)
    %
    % Reads the matrix stored in binary matrix file produced by
    % MVL Stereo Toolbox. Supports both uncompressed (old) dumps and 
    % compressed (new) dumps.
    %
    % Input:
    %  - filename: file name of binary matrix dump
    %
    % Output:
    %  - matrix: matrix stored in binary matrix dump
    %
    % (C) 2014, Rok Mandeljc <rok.mandeljc@fe.uni-lj.si>

    % Read raw data
    fid = fopen(filename, 'r', 'ieee-le');

    sig = fread(fid, 4, 'uint8=>char');
    switch sig',
        case 'BMD ',
            matrix = read_binary_matrix_dump_raw(fid);
        case 'BMDC',
            matrix = read_binary_matrix_dump_compressed(fid);
        otherwise
            error('Invalid matrix dump file!');
    end
    
    fclose(fid);
end

function matrix = read_binary_matrix_dump_raw (fid)
    % matrix = READ_BINARY_MATRIX_DUMP_RAW (fid)
    %
    % Reads raw (uncompressed) matrix dump from file descriptor.
    
    % Matrix header
    width = fread(fid, 1, 'uint32');
    height = fread(fid, 1, 'uint32');
    channels = fread(fid, 1, 'uint16');
    type = fread(fid, 1, 'uint16');

    % Matrix raw data
    raw_data = fread(fid, inf, 'uint8=>uint8');

    % Process data
    matrix = reshape_matrix_data (raw_data, width, height, channels, type);
end

function matrix = read_binary_matrix_dump_compressed (fid)
    % matrix = READ_BINARY_MATRIX_DUMP_COMPRESSED (fid)
    %
    % Reads compressed matrix dump from file descriptor.
    
    % Read compressed blob size
    blob_size = fread(fid, 1, 'uint32');
    
    % Read compressed blob
    blob = fread(fid, blob_size, 'uint8=>uint8');
    blob(1:4) = []; % Strip qCompress header (uncompressed file size)
            
    % Inflate
    raw_data = inflate(blob);
    
    % Matrix header
    width = typecast(raw_data(1:4), 'uint32');
    height = typecast(raw_data(5:8), 'uint32');
    channels = typecast(raw_data(9:10), 'uint16');
    type = typecast(raw_data(11:12), 'uint16');
    
    % Matrix raw data
    raw_data = raw_data(13:end);
    
    % Process data
    matrix = reshape_matrix_data (raw_data, width, height, channels, type);
end

function matrix = reshape_matrix_data (raw_data, width, height, channels, type)
    % matrix = RESHAPE_MATRIX_DATA (raw_data, width, height, channels, type)
    %
    % Reshapes and recasts the raw data into Matlab-compatible matrix.
    
    switch type,
        case 0,
            % CV_8U
            data = typecast(raw_data, 'uint8');
        case 1,
            % CV_8S
            data = typecast(raw_data, 'int8');
        case 2,
            % CV_16U
            data = typecast(raw_data, 'uint16');
        case 3,
            % CV_16S
            data = typecast(raw_data, 'int16');
        case 4,
            % CV_32S
            data = typecast(raw_data, 'int32');
        case 5,
            % CV_32F
            data = typecast(raw_data, 'single');
        case 6,
            % CV_64F
            data = typecast(raw_data, 'double');
        otherwise,
            error('Unhandled type %d', type);
    end

    matrix = reshape(data, [channels, width, height]);
    matrix = permute(matrix, [ 3 2 1 ]);
end

function uncompressed = inflate (compressed)
    % uncompressed = INFLATE (compressed)
    %
    % Uncompress zlib (deflate)-compressed byte array.
    
    assert(isa(compressed, 'uint8') && isvector(compressed), 'Input must be an uint8 vector!');

    inflater = java.util.zip.InflaterInputStream(java.io.ByteArrayInputStream(compressed));
    buffer = java.io.ByteArrayOutputStream();
    org.apache.commons.io.IOUtils.copy(inflater, buffer);
    inflater.close();
    
    uncompressed = typecast(buffer.toByteArray(), 'uint8')';
    buffer.close();
end
