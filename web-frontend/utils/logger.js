const fs = require('fs');
const path = require('path');

class Logger {
    constructor() {
        this.logDir = path.join(__dirname, '../logs');
        this.ensureLogDir();
    }

    ensureLogDir() {
        if (!fs.existsSync(this.logDir)) {
            fs.mkdirSync(this.logDir, { recursive: true });
        }
    }

    formatMessage(level, message, meta = {}) {
        const timestamp = new Date().toISOString();
        const metaStr = Object.keys(meta).length > 0 ? ` [${JSON.stringify(meta)}]` : '';
        return `[${timestamp}] [${level}] ${message}${metaStr}\n`;
    }

    writeToFile(filename, level, message, meta) {
        const logPath = path.join(this.logDir, filename);
        const formattedMessage = this.formatMessage(level, message, meta);
        
        fs.appendFile(logPath, formattedMessage, (err) => {
            if (err) {
                console.error('Failed to write to log file:', err);
            }
        });
    }

    // 应用程序日志
    info(message, meta = {}) {
        this.writeToFile('application.log', 'INFO', message, meta);
        if (process.env.NODE_ENV !== 'production') {
            console.log(`[INFO] ${message}`, meta);
        }
    }

    debug(message, meta = {}) {
        if (process.env.NODE_ENV !== 'production') {
            this.writeToFile('application.log', 'DEBUG', message, meta);
            console.debug(`[DEBUG] ${message}`, meta);
        }
    }

    warn(message, meta = {}) {
        this.writeToFile('application.log', 'WARN', message, meta);
        console.warn(`[WARN] ${message}`, meta);
    }

    error(message, meta = {}) {
        this.writeToFile('error.log', 'ERROR', message, meta);
        this.writeToFile('application.log', 'ERROR', message, meta);
        console.error(`[ERROR] ${message}`, meta);
    }

    fatal(message, meta = {}) {
        this.writeToFile('error.log', 'FATAL', message, meta);
        this.writeToFile('application.log', 'FATAL', message, meta);
        console.error(`[FATAL] ${message}`, meta);
    }

    // 访问日志
    access(req, res, responseTime) {
        const message = `${req.method} ${req.url} ${res.statusCode} ${responseTime}ms`;
        const meta = {
            ip: req.ip,
            userAgent: req.get('User-Agent'),
            userId: req.user ? req.user.id : null,
            username: req.user ? req.user.username : null
        };
        this.writeToFile('access.log', 'INFO', message, meta);
    }

    // WebSocket 日志
    websocket(message, meta = {}) {
        this.writeToFile('websocket.log', 'INFO', message, meta);
        if (process.env.NODE_ENV !== 'production') {
            console.log(`[WS] ${message}`, meta);
        }
    }

    // 认证日志
    auth(message, meta = {}) {
        this.writeToFile('auth.log', 'INFO', message, meta);
        this.info(`[AUTH] ${message}`, meta);
    }

    // 票务操作日志
    ticket(message, meta = {}) {
        this.writeToFile('ticket.log', 'INFO', message, meta);
        this.info(`[TICKET] ${message}`, meta);
    }

    // 管理员操作日志
    admin(message, meta = {}) {
        this.writeToFile('admin.log', 'INFO', message, meta);
        this.info(`[ADMIN] ${message}`, meta);
    }

    // 清理旧日志文件
    cleanup() {
        const maxAge = 30 * 24 * 60 * 60 * 1000; // 30 天
        const now = Date.now();

        fs.readdir(this.logDir, (err, files) => {
            if (err) {
                this.error('Failed to read log directory', { error: err.message });
                return;
            }

            files.forEach(file => {
                if (file === '.gitkeep' || file === 'README.md') return;
                
                const filePath = path.join(this.logDir, file);
                fs.stat(filePath, (err, stats) => {
                    if (err) return;
                    
                    if (now - stats.mtime.getTime() > maxAge) {
                        fs.unlink(filePath, (err) => {
                            if (!err) {
                                this.info(`Cleaned up old log file: ${file}`);
                            }
                        });
                    }
                });
            });
        });
    }

    // 获取日志统计信息
    getStats(callback) {
        fs.readdir(this.logDir, (err, files) => {
            if (err) {
                callback(err, null);
                return;
            }

            const stats = {
                totalFiles: 0,
                totalSize: 0,
                files: []
            };

            let pending = files.length;
            if (pending === 0) {
                callback(null, stats);
                return;
            }

            files.forEach(file => {
                if (file === '.gitkeep' || file === 'README.md') {
                    pending--;
                    if (pending === 0) callback(null, stats);
                    return;
                }

                const filePath = path.join(this.logDir, file);
                fs.stat(filePath, (err, fileStat) => {
                    if (!err) {
                        stats.totalFiles++;
                        stats.totalSize += fileStat.size;
                        stats.files.push({
                            name: file,
                            size: fileStat.size,
                            modified: fileStat.mtime
                        });
                    }
                    
                    pending--;
                    if (pending === 0) {
                        stats.files.sort((a, b) => b.modified - a.modified);
                        callback(null, stats);
                    }
                });
            });
        });
    }
}

// 创建单例实例
const logger = new Logger();

// 定期清理日志文件 (每天执行一次)
setInterval(() => {
    logger.cleanup();
}, 24 * 60 * 60 * 1000);

module.exports = logger;
