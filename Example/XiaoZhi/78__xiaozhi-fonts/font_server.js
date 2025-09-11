const fs = require('fs');

class LvglFont {
    constructor(binPath) {
        this.binPath = binPath;
        this.buffer = null;
        this.header = null;
        this.cmaps = [];
        this.glyphOffsets = [];
        this.glyphDescs = [];
        this.glyphBitmap = null;
        this.loaded = false;
    }

    loadBin() {
        try {
            this.buffer = fs.readFileSync(this.binPath);
            console.log(`Loaded font file: ${this.binPath}, size: ${this.buffer.length} bytes`);
            
            this._parseHeader();
            this._parseCmaps();
            this._parseLoca();
            this._parseGlyph();
            
            this.loaded = true;
            console.log('Font loaded successfully');
            return true;
        } catch (error) {
            console.error('Error loading font:', error);
            return false;
        }
    }

    _parseHeader() {
        let offset = 0;
        
        // 读取 "head" 标签长度
        const headLength = this._readLabel(offset, 'head');
        if (headLength < 0) {
            throw new Error('Invalid head section');
        }
        offset += 8; // label长度(4) + "head"(4)

        // 解析字体头部
        this.header = {
            version: this.buffer.readUInt32LE(offset),
            tablesCount: this.buffer.readUInt16LE(offset + 4),
            fontSize: this.buffer.readUInt16LE(offset + 6),
            ascent: this.buffer.readUInt16LE(offset + 8),
            descent: this.buffer.readInt16LE(offset + 10),
            typoAscent: this.buffer.readUInt16LE(offset + 12),
            typoDescent: this.buffer.readInt16LE(offset + 14),
            typoLineGap: this.buffer.readUInt16LE(offset + 16),
            minY: this.buffer.readInt16LE(offset + 18),
            maxY: this.buffer.readInt16LE(offset + 20),
            defaultAdvanceWidth: this.buffer.readUInt16LE(offset + 22),
            kerningScale: this.buffer.readUInt16LE(offset + 24),
            indexToLocFormat: this.buffer.readUInt8(offset + 26),
            glyphIdFormat: this.buffer.readUInt8(offset + 27),
            advanceWidthFormat: this.buffer.readUInt8(offset + 28),
            bitsPerPixel: this.buffer.readUInt8(offset + 29),
            xyBits: this.buffer.readUInt8(offset + 30),
            whBits: this.buffer.readUInt8(offset + 31),
            advanceWidthBits: this.buffer.readUInt8(offset + 32),
            compressionId: this.buffer.readUInt8(offset + 33),
            subpixelsMode: this.buffer.readUInt8(offset + 34),
            padding: this.buffer.readUInt8(offset + 35),
            underlinePosition: this.buffer.readInt16LE(offset + 36),
            underlineThickness: this.buffer.readUInt16LE(offset + 38)
        };

        this.cmapsStart = headLength;
        console.log('Header parsed:', this.header);
    }

    _parseCmaps() {
        let offset = this.cmapsStart;
        
        const cmapsLength = this._readLabel(offset, 'cmap');
        if (cmapsLength < 0) {
            throw new Error('Invalid cmap section');
        }
        offset += 8;

        const cmapsSubtablesCount = this.buffer.readUInt32LE(offset);
        offset += 4;

        console.log(`Found ${cmapsSubtablesCount} cmap subtables`);

        // 读取 cmap 表描述符
        const cmapTables = [];
        for (let i = 0; i < cmapsSubtablesCount; i++) {
            cmapTables.push({
                dataOffset: this.buffer.readUInt32LE(offset),
                rangeStart: this.buffer.readUInt32LE(offset + 4),
                rangeLength: this.buffer.readUInt16LE(offset + 8),
                glyphIdStart: this.buffer.readUInt16LE(offset + 10),
                dataEntriesCount: this.buffer.readUInt16LE(offset + 12),
                formatType: this.buffer.readUInt8(offset + 14),
                padding: this.buffer.readUInt8(offset + 15)
            });
            offset += 16;
        }

        // 解析每个 cmap 子表
        for (let i = 0; i < cmapsSubtablesCount; i++) {
            const table = cmapTables[i];
            const cmap = {
                rangeStart: table.rangeStart,
                rangeLength: table.rangeLength,
                glyphIdStart: table.glyphIdStart,
                type: table.formatType,
                unicodeList: null,
                glyphIdOfsList: null,
                listLength: 0
            };

            const dataOffset = this.cmapsStart + table.dataOffset;

            switch (table.formatType) {
                case 0: // LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
                    cmap.glyphIdOfsList = [];
                    for (let j = 0; j < table.dataEntriesCount; j++) {
                        cmap.glyphIdOfsList.push(this.buffer.readUInt8(dataOffset + j));
                    }
                    cmap.listLength = cmap.rangeLength;
                    break;

                case 1: // LV_FONT_FMT_TXT_CMAP_SPARSE_FULL
                case 3: // LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
                    cmap.unicodeList = [];
                    let listOffset = dataOffset;
                    for (let j = 0; j < table.dataEntriesCount; j++) {
                        // Unicode 值需要加上范围起始值
                        const unicodeOffset = this.buffer.readUInt16LE(listOffset);
                        cmap.unicodeList.push(unicodeOffset + table.rangeStart);
                        listOffset += 2;
                    }
                    cmap.listLength = table.dataEntriesCount;

                    if (table.formatType === 1) { // SPARSE_FULL
                        cmap.glyphIdOfsList = [];
                        for (let j = 0; j < table.dataEntriesCount; j++) {
                            cmap.glyphIdOfsList.push(this.buffer.readUInt16LE(listOffset));
                            listOffset += 2;
                        }
                    }
                    break;

                case 2: // LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
                    // FORMAT0_TINY: 连续范围，无额外数据
                    cmap.listLength = cmap.rangeLength;
                    break;

                default:
                    console.warn(`Unknown cmap format type: ${table.formatType}`);
                    continue;
            }

            this.cmaps.push(cmap);
        }

        this.locaStart = this.cmapsStart + cmapsLength;
        console.log(`Parsed ${this.cmaps.length} cmaps`);
    }

    _parseLoca() {
        let offset = this.locaStart;
        
        const locaLength = this._readLabel(offset, 'loca');
        if (locaLength < 0) {
            throw new Error('Invalid loca section');
        }
        offset += 8;

        const locaCount = this.buffer.readUInt32LE(offset);
        offset += 4;

        console.log(`Found ${locaCount} glyph locations`);

        this.glyphOffsets = [];
        if (this.header.indexToLocFormat === 0) {
            // 16-bit offsets
            for (let i = 0; i < locaCount; i++) {
                this.glyphOffsets.push(this.buffer.readUInt16LE(offset));
                offset += 2;
            }
        } else if (this.header.indexToLocFormat === 1) {
            // 32-bit offsets
            for (let i = 0; i < locaCount; i++) {
                this.glyphOffsets.push(this.buffer.readUInt32LE(offset));
                offset += 4;
            }
        } else {
            throw new Error(`Unknown index_to_loc_format: ${this.header.indexToLocFormat}`);
        }

        this.glyphStart = this.locaStart + locaLength;
    }

    _parseGlyph() {
        let offset = this.glyphStart;
        
        const glyphLength = this._readLabel(offset, 'glyf');
        if (glyphLength < 0) {
            throw new Error('Invalid glyf section');
        }
        offset += 8;

        console.log(`Parsing ${this.glyphOffsets.length} glyphs`);

        this.glyphDescs = new Array(this.glyphOffsets.length);
        let totalBitmapSize = 0;

        // 第一遍：解析所有 glyph 描述符
        for (let i = 0; i < this.glyphOffsets.length; i++) {
            const glyphOffset = this.glyphStart + this.glyphOffsets[i];
            const bitReader = new BitReader(this.buffer, glyphOffset);

            const desc = {
                advW: 0,
                ofsX: 0,
                ofsY: 0,
                boxW: 0,
                boxH: 0,
                bitmapIndex: 0
            };

            if (this.header.advanceWidthBits === 0) {
                desc.advW = this.header.defaultAdvanceWidth;
            } else {
                desc.advW = bitReader.readBits(this.header.advanceWidthBits);
            }

            if (this.header.advanceWidthFormat === 0) {
                desc.advW *= 16; // LVGL 使用 1/16 像素单位
            }

            desc.ofsX = bitReader.readSignedBits(this.header.xyBits);
            desc.ofsY = bitReader.readSignedBits(this.header.xyBits);
            desc.boxW = bitReader.readBits(this.header.whBits);
            desc.boxH = bitReader.readBits(this.header.whBits);

            // 第一个 glyph (index 0) 通常是缺失字符的占位符
            if (i === 0) {
                desc.advW = 0;
                desc.boxW = 0;
                desc.boxH = 0;
                desc.ofsX = 0;
                desc.ofsY = 0;
            }

            // 计算 bitmap 大小
            const nbits = this.header.advanceWidthBits + 2 * this.header.xyBits + 2 * this.header.whBits;
            const nextOffset = (i < this.glyphOffsets.length - 1) ? this.glyphOffsets[i + 1] : glyphLength;
            const bmpSize = nextOffset - this.glyphOffsets[i] - Math.floor(nbits / 8);

            desc.bitmapIndex = totalBitmapSize;
            if (desc.boxW * desc.boxH !== 0) {
                totalBitmapSize += bmpSize;
            }

            this.glyphDescs[i] = desc;
        }

        // 第二遍：提取所有 bitmap 数据
        this.glyphBitmap = Buffer.alloc(totalBitmapSize);
        let currentBitmapOffset = 0;

        for (let i = 1; i < this.glyphOffsets.length; i++) { // 跳过第一个占位符
            const desc = this.glyphDescs[i];
            if (desc.boxW * desc.boxH === 0) continue;

            const glyphOffset = this.glyphStart + this.glyphOffsets[i];
            const bitReader = new BitReader(this.buffer, glyphOffset);

            // 跳过描述符部分
            const nbits = this.header.advanceWidthBits + 2 * this.header.xyBits + 2 * this.header.whBits;
            bitReader.readBits(nbits);

            // 读取 bitmap 数据
            const nextOffset = (i < this.glyphOffsets.length - 1) ? this.glyphOffsets[i + 1] : glyphLength;
            const bmpSize = nextOffset - this.glyphOffsets[i] - Math.floor(nbits / 8);

            if (nbits % 8 === 0) {
                // 字节对齐，直接复制
                const srcOffset = glyphOffset + Math.floor(nbits / 8);
                this.buffer.copy(this.glyphBitmap, currentBitmapOffset, srcOffset, srcOffset + bmpSize);
            } else {
                // 非字节对齐，需要逐位读取
                for (let k = 0; k < bmpSize - 1; k++) {
                    this.glyphBitmap[currentBitmapOffset + k] = bitReader.readBits(8);
                }
                const lastByte = bitReader.readBits(8 - (nbits % 8));
                this.glyphBitmap[currentBitmapOffset + bmpSize - 1] = lastByte << (nbits % 8);
            }

            currentBitmapOffset += bmpSize;
        }

        console.log(`Extracted ${totalBitmapSize} bytes of bitmap data`);
    }

    _readLabel(offset, expectedLabel) {
        if (offset + 8 > this.buffer.length) return -1;
        
        const length = this.buffer.readUInt32LE(offset);
        const label = this.buffer.subarray(offset + 4, offset + 8).toString('ascii');
        
        if (label !== expectedLabel) {
            throw new Error(`Expected label '${expectedLabel}', got '${label}'`);
        }
        
        return length;
    }



    // Unicode 码点转 glyph ID
    getGlyphId(codepoint) {
        for (let i = 0; i < this.cmaps.length; i++) {
            const cmap = this.cmaps[i];
            
            let glyphId = 0;

            switch (cmap.type) {
                case 0: // FORMAT0_FULL
                    if (codepoint >= cmap.rangeStart && codepoint < cmap.rangeStart + cmap.rangeLength) {
                        const index = codepoint - cmap.rangeStart;
                        glyphId = cmap.glyphIdStart + (cmap.glyphIdOfsList ? cmap.glyphIdOfsList[index] : index);
                    }
                    break;

                case 1: // SPARSE_FULL
                    if (cmap.unicodeList) {
                        const fullIndex = cmap.unicodeList.indexOf(codepoint);
                        if (fullIndex >= 0) {
                            glyphId = cmap.glyphIdStart + (cmap.glyphIdOfsList ? cmap.glyphIdOfsList[fullIndex] : fullIndex);
                        }
                    }
                    break;

                case 2: // FORMAT0_TINY
                    if (codepoint >= cmap.rangeStart && codepoint < cmap.rangeStart + cmap.rangeLength) {
                        glyphId = cmap.glyphIdStart + (codepoint - cmap.rangeStart);
                    }
                    break;

                case 3: // SPARSE_TINY
                    if (cmap.unicodeList) {
                        const tinyIndex = cmap.unicodeList.indexOf(codepoint);
                        if (tinyIndex >= 0) {
                            glyphId = cmap.glyphIdStart + tinyIndex;
                        }
                    }
                    break;

                default:
                    continue;
            }

            if (glyphId > 0 && glyphId < this.glyphDescs.length) {
                return glyphId;
            }
        }
        
        return 0; // 返回缺失字符
    }

    // 获取指定文字的 glyph 描述符和 bitmap
    getGlyphData(codepoint) {
        if (!this.loaded) {
            throw new Error('Font not loaded. Call loadBin() first.');
        }

        const glyphId = this.getGlyphId(codepoint);
        if (glyphId === 0 || glyphId >= this.glyphDescs.length) {
            return null; // 字符不存在
        }

        const desc = this.glyphDescs[glyphId];
        
        // 提取 bitmap 数据
        let bitmap = null;
        if (desc.boxW > 0 && desc.boxH > 0) {
            const bitmapSize = this._calculateBitmapSize(desc.boxW, desc.boxH);
            const endIndex = desc.bitmapIndex + bitmapSize;
            bitmap = this.glyphBitmap.subarray(desc.bitmapIndex, endIndex);
        }

        return {
            codepoint: codepoint,
            glyphId: glyphId,
            desc: {
                advW: desc.advW,
                boxW: desc.boxW,
                boxH: desc.boxH,
                ofsX: desc.ofsX,
                ofsY: desc.ofsY
            },
            bitmap: bitmap,
            bitmapLength: bitmap ? bitmap.length : 0
        };
    }

    // 批量获取多个文字的 glyph 数据
    getMultipleGlyphData(codepoints) {
        const results = [];
        for (const cp of codepoints) {
            const data = this.getGlyphData(cp);
            if (data) {
                results.push(data);
            }
        }
        return results;
    }

    _calculateBitmapSize(boxW, boxH) {
        const bpp = this.header.bitsPerPixel;
        const pixelCount = boxW * boxH;
        return Math.ceil((pixelCount * bpp) / 8);
    }

    // 获取字体信息
    getFontInfo() {
        if (!this.loaded) return null;
        
        return {
            fontSize: this.header.fontSize,
            ascent: this.header.ascent,
            descent: this.header.descent,
            lineHeight: this.header.ascent - this.header.descent,
            baseLine: -this.header.descent,
            bitsPerPixel: this.header.bitsPerPixel,
            underlinePosition: this.header.underlinePosition,
            underlineThickness: this.header.underlineThickness
        };
    }
}

// 辅助类：位读取器
class BitReader {
    constructor(buffer, startOffset = 0) {
        this.buffer = buffer;
        this.byteOffset = startOffset;
        this.bitOffset = 0;
        this.currentByte = 0;
        this.validBits = 0;
    }

    readBits(n) {
        let result = 0;
        
        for (let i = 0; i < n; i++) {
            if (this.validBits === 0) {
                if (this.byteOffset >= this.buffer.length) {
                    throw new Error('Buffer overflow in BitReader');
                }
                this.currentByte = this.buffer[this.byteOffset++];
                this.validBits = 8;
            }

            const bit = (this.currentByte >> (this.validBits - 1)) & 1;
            result = (result << 1) | bit;
            this.validBits--;
        }

        return result;
    }

    readSignedBits(n) {
        const value = this.readBits(n);
        // 检查符号位
        if (value & (1 << (n - 1))) {
            // 负数：扩展符号位
            return value | (~0 << n);
        }
        return value;
    }
}

if (!module.parent) {
    const font = new LvglFont('build/full_AlibabaPuHuiTi-3-55-Regular_20_4.bin');
    if (font.loadBin()) {
        console.log('Font info:', font.getFontInfo());
        
        // 简单的 API 接口测试
        console.log('\n=== API 接口测试 ===');
        
        // 测试一些字符
        const testChars = ['A', 'a', '中', '欸', 'ㄅ'];
        console.log('单个字符查询测试:');
        for (const char of testChars) {
            const codepoint = char.codePointAt(0);
            const data = font.getGlyphData(codepoint);
            if (data) {
                console.log(`  ✓ '${char}' -> 字形ID: ${data.glyphId}, 尺寸: ${data.desc.boxW}x${data.desc.boxH}px`);
            } else {
                console.log(`  ✗ '${char}' -> 未找到`);
            }
        }
        
        // 批量查询测试
        const codepoints = testChars.map(c => c.codePointAt(0));
        const batchResult = font.getMultipleGlyphData(codepoints);
        console.log(`\n批量查询: ${batchResult.length}/${testChars.length} 个字符找到`);
        
        // 字体信息
        const fontInfo = font.getFontInfo();
        console.log(`字体信息: ${fontInfo.fontSize}px, ${fontInfo.bitsPerPixel}bpp, 行高${fontInfo.lineHeight}px`);
        
        console.log('\n字体解析器就绪 ✓');
    }
}

module.exports = LvglFont;
