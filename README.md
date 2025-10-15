# Maum (마음) - Classic BBS Revival

> **마음** (maum) means "heart" or "mind" in Korean - the heart of community communication

Maum is a modern implementation of classic-style Bulletin Board System (BBS) accessible through TELNET and SSH. It revives the golden age of Korean PC communication services while embracing contemporary security and accessibility standards.

## 🎯 Concept

### What is a BBS?

A Bulletin Board System (BBS) is a computer server running software that allows users to connect via terminal to:
- Read and post messages on topic-based bulletin boards
- Participate in discussion forums organized by subject
- Download and upload files
- Send private messages to other users
- Navigate through hierarchical menu systems
- Engage in real-time chat rooms

Unlike modern web forums or simple chat applications, classic BBS systems offered a complete community experience through a text-based interface.

### Inspiration: Classic Korean PC Communication

Maum draws inspiration from legendary Korean PC communication services that defined online culture in Korea during the 1990s:

#### 🌐 Nownuri (나우누리, 1994-2022)
- One of Korea's first commercial online services
- Featured organized bulletin boards (게시판) by topic
- Community forums for hobbies, technology, and social discussions
- File exchange libraries
- Real-time chat rooms (대화방)
- Hierarchical navigation with numbered menu selections

#### 📡 Hitel (하이텔, 1991-2007)
- Pioneer of Korean online communication
- Text-based interface accessed via modem
- Rich community structure with special interest groups (SIG)
- Message boards organized by category
- File libraries and shareware distribution
- Chat and messaging systems

These services were more than just communication tools - they were **digital communities** where people formed lasting relationships, shared knowledge, and created culture.

### Global BBS Heritage

Maum also acknowledges the broader BBS tradition:

#### Classic BBS (1978-1990s)
- Door games and text adventures
- ANSI art and creative text formatting
- FidoNet message networks
- File sharing and software distribution
- SysOp (System Operator) managed communities

#### Modern Variations
- **4chan/Textboards**: Board-based structure with anonymous posting
- **SSH Chat**: Real-time terminal chat (like ssh-chatter)
- **Gopher**: Hierarchical document system

**Important**: Maum is a **full BBS**, not just a chat system. While projects like [ssh-chatter](https://github.com/gg582/ssh-chatter) provide real-time chat, Maum offers the complete BBS experience: boards, messages, files, menus, and chat combined.

## 🌟 Core Features (Planned)

### 1. Traditional BBS Interface
- **Menu-driven navigation**: Hierarchical text menus (Main → Boards → Forums → Topics)
- **ANSI/UTF-8 support**: Colorful text art and Korean character support
- **Command-line interface**: Keyboard shortcuts and commands
- **Session management**: User login, profiles, and preferences

### 2. Bulletin Boards (게시판)
- **Topic-based boards**: Technology, hobbies, announcements, general discussion
- **Threaded conversations**: Reply chains and discussion threads
- **Message persistence**: Posts saved and browsable
- **Search functionality**: Find posts and topics

### 3. File Areas
- **Upload/Download**: Share files within the community
- **File descriptions**: Browse and search file libraries
- **Categorization**: Organized file sections

### 4. Communication
- **Public boards**: Community discussions
- **Private messages**: Direct user-to-user communication
- **Real-time chat**: Live chat rooms for instant conversation
- **Announcements**: System-wide bulletins

### 5. User System
- **Authentication**: Secure login with SSH keys or passwords
- **User profiles**: Customizable profiles and signatures
- **Access levels**: User, moderator, SysOp roles
- **Activity tracking**: Post counts, last login, etc.

### 6. Modern Security
- **SSH protocol**: Encrypted connections (preferred)
- **TELNET support**: Legacy access option
- **Authentication**: Secure credential management
- **SSL/TLS**: Optional encrypted TELNET

## 🏗️ Technical Architecture

### Access Methods

```
┌─────────────────────────────────────────┐
│         Maum BBS Server                 │
├─────────────────────────────────────────┤
│  SSH (Port 2222) ←  Preferred/Secure    │
│  TELNET (Port 23) ← Legacy/Optional     │
└─────────────────────────────────────────┘
           ↓           ↓
    [Terminal]    [Terminal]
     Client        Client
```

### User Experience Flow

```
Connection → Login/Register → Main Menu
                                  ↓
    ┌─────────────────────────────┼─────────────────────────────┐
    ↓                             ↓                             ↓
Bulletin Boards              File Areas                   Chat Rooms
    ↓                             ↓                             ↓
Browse/Post                  Upload/Download               Real-time Chat
    ↓                             ↓                             ↓
Read Messages                View Files                    Send Messages
    ↓
Reply/Comment
```

### Design Principles

1. **Nostalgic UX**: Recreate the feel of 1990s Korean PC communication
2. **Modern Backend**: Use contemporary protocols and security
3. **Terminal-First**: Optimized for terminal/console access
4. **Korean Support**: Full UTF-8 Korean language support
5. **Community-Centric**: Tools for building and managing communities
6. **Extensible**: Plugin/module system for features

## 🎨 Interface Concept

### Main Menu Example
```
╔════════════════════════════════════════════════════════════╗
║                   마음 (Maum) BBS                          ║
║              Classic Korean BBS Revival                     ║
╠════════════════════════════════════════════════════════════╣
║  Welcome, User123!                    Last login: 2025-01-15║
╠════════════════════════════════════════════════════════════╣
║                                                              ║
║  [1] Bulletin Boards (게시판)                               ║
║  [2] File Libraries (자료실)                                ║
║  [3] Chat Rooms (대화방)                                    ║
║  [4] Private Messages (쪽지함) [3 new]                      ║
║  [5] User Profile (내 정보)                                 ║
║  [6] System Info (시스템 정보)                              ║
║  [7] Help (도움말)                                          ║
║  [Q] Quit (종료)                                            ║
║                                                              ║
╚════════════════════════════════════════════════════════════╝

Select [1-7, Q]: _
```

### Board View Example
```
╔════════════════════════════════════════════════════════════╗
║  Technology Board (기술 게시판)                             ║
╠════════════════════════════════════════════════════════════╣
║  No  │ Title              │ Author   │ Date     │ Replies  ║
╠═════════════════════════════════════════════════════════════╣
║  125 │ New SSH tips       │ alice    │ 01/15    │ 3        ║
║  124 │ Terminal setup     │ bob      │ 01/14    │ 7        ║
║  123 │ BBS nostalgia      │ charlie  │ 01/13    │ 12       ║
║  122 │ Korean fonts       │ david    │ 01/12    │ 5        ║
╚════════════════════════════════════════════════════════════╝

[N]ew Post  [R]ead  [S]earch  [B]ack: _
```

## 🚀 Getting Started (Future)

### As a User
```bash
# Connect via SSH (secure, recommended)
ssh user@maum-bbs.example.com -p 2222

# Or via TELNET (legacy)
telnet maum-bbs.example.com
```

### Running Your Own Server
```bash
# Install dependencies
npm install -g maum-bbs

# Initialize server
maum-bbs init

# Start server
maum-bbs start --ssh-port 2222 --telnet-port 23

# Configure boards and settings
maum-bbs config
```

## 🎯 Project Goals

1. **Preserve History**: Keep the spirit of classic Korean BBS alive
2. **Modern Implementation**: Use current best practices and security
3. **Community Building**: Provide tools for creating digital communities
4. **Education**: Teach younger generations about BBS culture
5. **Open Source**: Share knowledge and enable others to run BBS systems

## 🛣️ Roadmap

### Phase 1: Foundation
- [ ] Core server architecture
- [ ] SSH/TELNET connection handling
- [ ] User authentication system
- [ ] Basic menu navigation
- [ ] Terminal rendering engine

### Phase 2: BBS Features
- [ ] Bulletin board system
- [ ] Message posting and threading
- [ ] File upload/download
- [ ] Private messaging
- [ ] User profiles

### Phase 3: Community
- [ ] Real-time chat integration
- [ ] Moderator tools
- [ ] Search functionality
- [ ] ANSI/UTF-8 art support
- [ ] Notification system

### Phase 4: Advanced
- [ ] Plugin system
- [ ] Inter-BBS networking (like FidoNet)
- [ ] Mobile terminal clients
- [ ] Web gateway (view-only)
- [ ] Activity analytics

## 🤝 Contributing

Maum is an open-source project welcoming contributions. Whether you remember the classic BBS era or are curious about digital history, your input is valued!

Areas for contribution:
- **Development**: Core features, protocols, UI/UX
- **Documentation**: Guides, tutorials, translations
- **Testing**: Security audits, usability testing
- **History**: Research on classic BBS systems
- **Community**: Moderation, content creation

## 📚 References & Inspiration

- **Nownuri (나우누리)**: Korea's classic PC communication service
- **Hitel (하이텔)**: Pioneer Korean online service
- **Chollian (천리안)**: Another major Korean PC communication platform
- **Classic BBS Systems**: The WELL, FidoNet, Citadel
- **Modern Projects**: ssh-chatter, Mystic BBS, Synchronet
- **Textboards**: 4chan, 2channel, textboard culture

## 📜 License

[To be determined]

## 💬 Contact

[To be determined]

---

**Note**: This project is currently in the concept/planning phase. The features and architecture described above represent the vision for Maum BBS. Implementation is ongoing.

*마음으로 연결되는 공간* - A space connected by heart
